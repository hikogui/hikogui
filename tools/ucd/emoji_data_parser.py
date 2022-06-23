
from .ucd_parser import parse_ucd

def parse_emoji_data(filename, descriptions):
    for columns in parse_ucd(filename):
        code_points = columns[0]
        t = columns[1]

        for code_point in code_points:
            d = descriptions[code_point]
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


