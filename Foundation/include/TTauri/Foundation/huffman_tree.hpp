


namespace TTauri {

class huffman_tree {
    using state_t = int const *;

    /** The internal data structure holding the tree.
     * A pair of values are added to the list for each tree-node.
     * The values have the following meaning:
     *  - negative number are relative offset from the current
     *    position to the next tree node. So that it is possible
     *    to simply add the negative value to the iterator.
     *  - positive number is the symbol value + 1.
     *  - zero was unused or not yet assigned.
     */
    std::vector<int> tree;

public:
    huffman_tree() {
        tree.push_back(0);
        tree.push_back(0);
    }

    /** Add a symbol to the huffman_tree.
     * 
     */
    void add(int symbol, int code, int code_length) noexcept {
        ttauri_assume(code_length >= 1);

        offset = 0;
        while (--code_length > 0) {
            int select = (code >> code_length) & 1;
            offset += select;
          
            auto &value = tree[offset];

            // value may not be a leaf.
            ttauri_assume(value < 0); 

            if (value == 0) {
                // Unused node entry. Point to the first of two new entries.
                value = -(ssize(tree) - offset);
                tree.push_back(0);
                tree.push_back(0);
                
            } else {
                // Entry is in the table, jump to the new entry.
                offset += -value;
            }
        }

        // place the symbol as a leaf.
        int select = code & 1;
        offset += select;

        auto &value = tree[offset];
        ttauri_assume(value == 0); 
        value = symbol + 1;
    }

    [[nodiscard]] state_t start() const noexcept {
        return tree.data();
    }

    /** Get a symbol from the huffman-tree.
     *
     * Before `get()` is called use `start()` to create a state to
     * pass between invocations. State is invalid after `get()` returns
     * a symbol or throws.
     *
     * @param code_bit The next bit from the huffman encoded stream.
     * @param state The state carried between invocations @see start().
     * @return Positive numbers are symbols, negative means more code_bits
     *         are needed.
     * @throw parse-error on invalid code-bit sequence.
     */
    [[nodiscard]] int get(bool code_bit, state_t &state) const {
        state += static_cast<ptrdiff_t>(code_bit);

        if (*state == 0) {
            TTAURI_THROW(parse_error("Code not in huffman tree."));
        }

        state -= static_cast<ptrdiff_t>(*state);
        return *state - 1;
    }
    
};



}
