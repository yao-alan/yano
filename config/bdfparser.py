""" This is a very simple parser that ignores many .bdf keywords.
"""

import os

FONT_FILE = 'boxxy.bdf'

font_bbox = {
    'width' : None,
    'height': None,
    'bl_x'  : None, # bottom-left x coordinate
    'bl_y'  : None  # bottom-right y coordinate
}
current_encoding = None
current_bbox = {
    'width' : None,
    'height': None,
    'bl_x'  : None,
    'bl_y'  : None
}

with open(f'./fonts/{FONT_FILE}') as f:
    if not os.path.exists(f'./.glyphs/{FONT_FILE.split(".")[0]}'):
        os.mkdir(f'./.glyphs/{FONT_FILE.split(".")[0]}')

    for line in f:
        tokens = line.split()

        if tokens[0] == 'FONTBOUNDINGBOX':
            font_bbox['width'] = tokens[1]
            font_bbox['height'] = tokens[2]
            font_bbox['bl_x'] = tokens[3]
            font_bbox['bl_y'] = tokens[4]

        elif tokens[0] == 'ENCODING':
            current_encoding = tokens[1]
        
        elif tokens[0] == 'BBX':
            current_bbox['width'] = tokens[1]
            current_bbox['height'] = tokens[2]
            current_bbox['bl_x'] = tokens[3]
            current_bbox['bl_y'] = tokens[4]

        elif tokens[0] == 'BITMAP':
            gf = open(f'./.glyphs/{FONT_FILE.split(".")[0]}/{current_encoding}.gbmp', 'w')
            gf.write(f'{font_bbox["width"]} {font_bbox["height"]} {font_bbox["bl_x"]} {font_bbox["bl_y"]}\n')
            gf.write(f'{current_bbox["width"]} {current_bbox["height"]} {current_bbox["bl_x"]} {current_bbox["bl_y"]}\n')

            tokens = f.readline().split()
            while tokens[0] != 'ENDCHAR':
                bits = bin(int(tokens[0], 16))[2:].zfill(8)
                gf.write(f'{bits}\n')
                tokens = f.readline().split()
            gf.close()

# generate cursor
