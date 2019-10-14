

#include <gsl/gsl>

namespace TTauri {
struct GraphemeBreakState;

class BinaryUnicodeData {
private:
    std::unique_ptr<ResourceView> view;

    gsl::span<std::byte const> bytes;

    gsl::span<BinaryUnicodeData_CodeUnit> codeUnits;
    gsl::span<BinaryUnicodeData_CanonicalComposition> canonicalCompositions;
public:
    /*! Load a true type font.
    * The methods in this class will parse the true-type font at run time.
    * This also means that the bytes passed into this constructor will need to
    * remain available.
    */
    BinaryUnicodeData(gsl::span<std::byte const> bytes) :
        bytes(bytes) {}

    BinaryUnicodeData(std::unique_ptr<ResourceView> view) :
        view(std::move(view)) {
        bytes = this->view->bytes();
    }


    BinaryUnicodeData() = delete;
    BinaryUnicodeData(BinaryUnicodeData const &other) = delete;
    BinaryUnicodeData &operator=(BinaryUnicodeData const &other) = delete;
    BinaryUnicodeData(BinaryUnicodeData &&other) = delete;
    BinaryUnicodeData &operator=(BinaryUnicodeData &&other) = delete;
    ~BinaryUnicodeData() = default;

    /*! This function will canonically decompose the text.
     * Ligatures will be decomposed.
     *
     * \param text to decompose, must be passed to checkCanonicalComposition() first.
     * \return The text after canonical decomposition.
     */
    std::u32string canonicalDecompose(std::u32string_view text, bool decomposeLigatures=true) const noexcept;

    /*! This function will compatible decompose the text.
    * This function should be used before comparing two texts.
    *
    * \param text to decompose, must be passed to checkCanonicalComposition() first.
    * \return The text after canonical decomposition.
    */
    std::u32string compatibleDecompose(std::u32string_view text) const noexcept;

    /*! This function will compose the text.
     *
     * \param text to compose, must be passed to checkCanonicalComposition() or canonicalDecompose() first.
     * \return The text after composition.
     */
    size_t compose(std::u32string &text) const noexcept;


private:
    BinaryUnicodeData_CodeUnit const *getCodeUnitInfo(char32_t c) const noexcept;

    bool checkGraphemeBreak(char32_t c, GraphemeBreakState &state) const noexcept;
    char32_t compose(char32_t startCharacter, char32_t composingCharacter) const noexcept;
    void decompose(std::u32string &result, char32_t c, bool canonical, bool decomposeLigatures) const noexcept;
    std::u32string decompose(std::u32string_view text, bool canonical, bool decomposeLigatures=false) const noexcept;
    void normalizeDecompositionOrder(std::u32string &result) const noexcept;

};

}