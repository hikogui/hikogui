
import sys

S_base = 0xac00
L_base = 0x1100
V_base = 0x1161
T_base = 0x11a7
L_count = 19;
V_count = 21;
T_count = 28;
N_count = V_count * T_count;
S_count = L_count * N_count;

def add_hangul_decompositions(descriptions):
    print("Add canonical Hangul decompositions.", file=sys.stderr, flush=True)

    for S_index in range(S_count):
        code_point = S_base + S_index
        d = descriptions[code_point]
        d.decomposition_type = None

        # Unicode standard chapter 3.12 "Hangul Syllable Decomposition"
        if S_index % T_count == 0:
            d.grapheme_cluster_break = "LV"
            d.line_break = "H2"

            L_index = S_index // N_count
            V_index = (S_index % N_count) // T_count
            L_part = L_base + L_index
            V_part = V_base + V_index
            d.decomposition_mapping = [L_part, V_part]

        else:
            d.grapheme_cluster_break = "LVT"
            d.line_break = "H3"

            LV_index = (S_index // T_count) * T_count
            T_index = S_index % T_count
            LV_part = S_base + LV_index
            T_part = T_base + T_index
            d.decomposition_mapping = [LV_part, T_part]
