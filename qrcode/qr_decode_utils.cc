#include "qrcode/qr_decode_utils.h"

#include "absl/types/span.h"

#include "qrcode/array_walker.h"

void UnmaskArray(const QRAttributes& attributes, QRCodeArray* array,
                 unsigned char mask_pattern) {
  typedef std::function<bool(const Point&)> Unmasker;
  static Unmasker unmaskers[8] = {
      [](const Point& p) { return (p.x + p.y) % 2 == 0; },          // 000
      [](const Point& p) { return p.x % 2 == 0; },                  // 001
      [](const Point& p) { return p.y % 3 == 0; },                  // 010
      [](const Point& p) { return (p.x + p.y) % 3 == 0; },          // 011
      [](const Point& p) { return (p.x / 2 + p.y / 3) % 2 == 0; },  // 100
      [](const Point& p) {
        return (p.x * p.y) % 2 + (p.x * p.y) % 3 == 0;
      },  // 101
      [](const Point& p) {
        return ((p.x * p.y) % 2 + (p.x * p.y) % 3) % 2 == 0;
      },  // 110
      [](const Point& p) {
        return ((p.x * p.y) % 3 + (p.x + p.y) % 2) % 2 == 0;
      },  // 111
  };

  Unmasker* unmasker = &unmaskers[mask_pattern];

  for (int y = 0; y < attributes.modules_per_side(); ++y) {
    for (int x = 0; x < attributes.modules_per_side(); ++x) {
      Point p(x, y);
      if (attributes.GetModuleType(p) != QRAttributes::TYPE_DATA) {
        continue;
      }

      array->Set(p, array->Get(p) ^ (*unmasker)(p));
    }
  }
}

std::vector<unsigned char> FindCodewords(const QRAttributes& attributes,
                                         const QRCodeArray& array) {
  ArrayWalker walker(attributes);
  std::vector<unsigned char> out;

  int num_bits = 0;
  unsigned char cur_val = 0;
  for (;;) {
    absl::optional<Point> p = walker.Next();
    if (!p.has_value()) {
      break;
    }

    cur_val = (cur_val << 1) | array.Get(*p);
    num_bits++;

    if (num_bits == 8) {
      out.push_back(cur_val);
      cur_val = 0;
      num_bits = 0;
    }
  }

  // We discard any bits left in cur_val because they're remainder
  // bits and aren't part of the QRCode.
  return out;
}

namespace {

// BlockState contains the current state for a given ECC block. We keep two
// instance for each *block* -- one each for data and ECC.
struct BlockState {
  BlockState(int total) : total(total), added(0) {}

  // Total number of codewords to expect for this block, as well as how many
  // have been added.
  int total, added;

  // Where to write codewords for this block.
  std::vector<unsigned char>* out;
};

void SplitGroup(absl::Span<const unsigned char> in,
                std::vector<BlockState>* block_states) {
  int cur_block = -1;
  for (int i = 0; i < in.size(); ++i) {
    BlockState* block_state;
    do {
      // Find the next block that hasn't read all of its codewords.
      cur_block = (cur_block + 1) % block_states->size();
      block_state = &(*block_states)[cur_block];
    } while (block_state->added >= block_state->total);

    (*block_state->out)[block_state->added] = in[i];
    ++block_state->added;
  }
}

}  // namespace

std::vector<CodewordBlock> SplitCodewordsIntoBlocks(
    const QRErrorLevelCharacteristics& error_characteristics,
    const std::vector<unsigned char>& unordered) {
  // Codewords arrive in the order they appear in the array, which means they're
  // grouped and they're interleaved. Data codewords appear first, followed by
  // the ECC codewords. Within each group, they're ordered by block (as
  // described in the Error Correction Characteristics table 13 in the 2000
  // spec).
  //
  // As an example, a V5H symbol has four blocks. The first two have 11 and 22
  // data and ECC codewords, respectively, while the second two have 12 and
  // 22. Encoding round-robins through the blocks, taking the next symbol from
  // each one. If a block is empty (which some will eventually be, as different
  // blocks have different numbers of codewords), that block is skipped.
  //
  // So if we have D1..D46, the blocks have D1-11, D12-22, D23-D33, and
  // D35-45. Each starts at the beginning of its range and returns each codeword
  // in turn. So for the first iteration each block contributes its first
  // element: D1, D12, D23, D35. And so on, until we get to the last full round
  // D11, D22, D33, and D45. The first two blocks are now empty (they only had
  // 11 data codewords apiece), so the next round is D34, D46.
  //
  // Then we repeat the whole process with the ECC blocks.
  //
  // Untangling this mess means reversing that process, first for data, then for
  // ECC.

  // Create one BlockState instance for each *block* (not block set) for both
  // data and ecc. A given BlackState will have the number of codewords
  // remaining for that block as well as a pointre to the right output array.
  std::vector<BlockState> data_states, ecc_states;

  for (const auto& block_set : error_characteristics.block_sets) {
    for (int i = 0; i < block_set.num_blocks; ++i) {
      data_states.emplace_back(block_set.data_codewords);
      ecc_states.emplace_back(block_set.block_codewords -
                              block_set.data_codewords);
    }
  }

  // Preallocate the codeword output arrays and put pointers to them in the
  // {data|ecc}_states vectors. We couldn't do this before because we didn't
  // know how big codeword_blocks needed to be (and thus couldn't guarantee
  // pointer stability).
  std::vector<CodewordBlock> codeword_blocks(data_states.size());
  for (int i = 0; i < data_states.size(); i++) {
    CodewordBlock* codeword_block = &codeword_blocks[i];

    codeword_block->data.resize(data_states[i].total);
    data_states[i].out = &codeword_blocks[i].data;

    codeword_block->ecc.resize(ecc_states[i].total);
    ecc_states[i].out = &codeword_blocks[i].ecc;
  }

  // Split the data codewords into their blocks.
  SplitGroup(absl::MakeConstSpan(unordered).subspan(
                 0, error_characteristics.total_data_codewords),
             &data_states);
  // Split the ECC codewords into their blocks.
  SplitGroup(absl::MakeConstSpan(unordered).subspan(
                 error_characteristics.total_data_codewords),
             &ecc_states);

  return codeword_blocks;
}
