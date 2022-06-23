
import parse_ucd
import description

def parse_emoji_data(filename):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        t = columns[1]

        if t == "Emoji":
        emoji = columns[1] == "Y"
        emoji_presentation = columns[2] == "Y"
        emoji_modifier = columns[3] == "Y"
        emoji_modifier_base = columns[4] == "Y"
        emoji_component = columns[5] == "Y"
        extended_pictographic = columns[6] == "Y"

        for code_point in code_points:
            d = description.descriptions[code_point]
            if t == "Emoji":
                d.emoji = True
            elif t == "Emoji_Presentation":
                d.emoji_presentation = True
            elif t == "Emoji_Modifier":
                d.emoji_modifier = True
            elif t == "Emoji_Modifier_Base":
                d.emoji_modifier_base = True
            elif t == "Emoji_Component":
                d.emoji_component = True
            elif t == "Extended_Pictographic":
                d.extended_pictographic = True
            else:
                raise RuntimeError("emoji-data")


