-- Copyright 2006-2020 Mitchell mitchell.att.foicica.com. See License.txt.
-- Notepad2e theme (based on original light theme) for Lua lexers.
-- Contributions by Ana Balan.

local property = require('lexer').property

-- Greyscale colors.
--property['color.dark_black'] = '#000000'
--property['color.black'] = '#1A1A1A'
property['color.light_black'] = '#333333'
--property['color.grey_black'] = '#4D4D4D'
--property['color.dark_grey'] = '#666666'
property['color.grey'] = '#808080'
--property['color.light_grey'] = '#999999'
--property['grey_white'] = '#B3B3B3'
property['color.dark_white'] = '#CCCCCC'
property['color.white'] = '#FFFFFF'
--property['color.light_white'] = '#FFFFFF'

-- Dark colors.
--property['color.dark_red'] = '#661A1A'
property['color.dark_yellow'] = '#66661A'
property['color.dark_green'] = '#1A661A'
--property['color.dark_teal'] = '#1A6666'
--property['color.dark_purple'] = '#661A66'
property['color.dark_orange'] = '#B3661A'
--property['color.dark_pink'] = '#B36666'
property['color.dark_lavender'] = '#6666B3'
property['color.dark_blue'] = '#1A66B3'

-- Normal colors.
property['color.red'] = '#994D4D'
property['color.yellow'] = '#99994D'
property['color.green'] = '#4D994D'
property['color.teal'] = '#4D9999'
property['color.purple'] = '#994D99'
--property['color.orange'] = '#E6994D'
--property['color.pink'] = '#E69999'
property['color.lavender'] = '#9999E6'
--property['color.blue'] = '#4D99E6'

-- Light colors.
property['color.light_red'] = '#C08080'
--property['color.light_yellow'] = '#CCCC80'
--property['color.light_green'] = '#80CC80'
--property['color.light_teal'] = '#80CCCC'
--property['color.light_purple'] = '#CC80CC'
--property['color.light_orange'] = '#FFCC80'
--property['color.light_pink'] = '#FFCCCC'
--property['color.light_lavender'] = '#CCCCFF'
property['color.light_blue'] = '#80CCFF'

-- Default style.
property['font'] = 'Lucida Console'
property['fontsize'] = '10'

-- Predefined styles.
property['style.default'] = 'fore:$(color.light_black),back:$(color.white)'
property['style.linenumber'] = 'fore:$(color.black),back:#F0F0F0'
property['style.bracelight'] = 'fore:$(color.light_blue)'
property['style.bracebad'] = 'fore:$(color.light_red)'
property['style.controlchar'] = ''
property['style.indentguide'] = 'fore:$(color.dark_white)'
property['style.calltip'] = 'fore:$(color.light_black)'
property['style.folddisplaytext'] = 'fore:$(color.white)'

-- Token styles.
property['style.class'] = 'fore:$(color.yellow)'
property['style.comment'] = 'fore:$(color.grey)'
property['style.constant'] = 'fore:$(color.red)'
property['style.embedded'] = '$(style.keyword),back:$(color.dark_white)'
property['style.error'] = 'fore:$(color.red),italics'
property['style.function'] = 'fore:$(color.dark_orange)'
property['style.identifier'] = ''
property['style.keyword'] = 'fore:$(color.dark_blue)'
property['style.label'] = 'fore:$(color.dark_orange)'
property['style.number'] = 'fore:$(color.teal)'
property['style.operator'] = 'fore:$(color.purple)'
property['style.preprocessor'] = 'fore:$(color.dark_yellow)'
property['style.regex'] = 'fore:$(color.dark_green)'
property['style.string'] = 'fore:$(color.green)'
property['style.type'] = 'fore:$(color.lavender)'
property['style.variable'] = 'fore:$(color.dark_lavender)'
property['style.whitespace'] = ''
