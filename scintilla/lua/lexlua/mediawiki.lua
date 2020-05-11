-- Copyright 2006-2019 Mitchell mitchell.att.foicica.com. See License.txt.
-- MediaWiki LPeg lexer.
-- Contributed by Alexander Misel.

local lexer = require('lexer')
local token, word_match = lexer.token, lexer.word_match
local P, R, S, B = lpeg.P, lpeg.R, lpeg.S, lpeg.B

local lex = lexer.new('mediawiki')

-- Comments.
lex:add_rule('comment', token(lexer.COMMENT, '<!--' * (lexer.any - '-->')^0 *
                                             P('-->')^-1))

-- HTML-like tags
local tag_start = token('tag_start', '<' * P('/')^-1 * lexer.alnum^1 *
                                     lexer.space^0)
local tag_attr = token('tag_attr', lexer.alpha^1 * lexer.space^0 *
                                   ('=' * lexer.space^0 *
                                    ('"' * ((lexer.any - S('>"\\')) +
                                     ('\\' * lexer.any))^0 * '"' +
                                     (lexer.any - lexer.space - '>')^0)^-1)^0 *
                                   lexer.space^0)
local tag_end = token('tag_end', P('/')^-1 * '>')
lex:add_rule('tag', tag_start * tag_attr^0 * tag_end)
lex:add_style('tag_start', lexer.STYLE_KEYWORD)
lex:add_style('tag_attr', lexer.STYLE_TYPE)
lex:add_style('tag_end', lexer.STYLE_KEYWORD)

-- Link
lex:add_rule('link', token(lexer.STRING, S('[]')))
lex:add_rule('internal_link', B('[[') *
                              token('link_article', (lexer.any - '|' - ']]')^1))
lex:add_style('link_article', lexer.STYLE_STRING..',underlined')

-- Templates and parser functions.
lex:add_rule('template', token(lexer.OPERATOR, S('{}')))
lex:add_rule('parser_func', B('{{') *
                            token('parser_func', P('#') * lexer.alpha^1 +
                                                 lexer.upper^1 * ':'))
lex:add_rule('template_name', B('{{') *
                              token('template_name', (lexer.any - S('{}|'))^1))
lex:add_style('parser_func', lexer.STYLE_FUNCTION)
lex:add_style('template_name', lexer.STYLE_OPERATOR..',underlined')

-- Operators.
lex:add_rule('operator', token(lexer.OPERATOR, S('-=|#~!')))

-- Behavior switches
local start_pat = P(function(_, pos) return pos == 1 end)
lex:add_rule('behavior_switch', (B(lexer.space) + start_pat) *
                                token('behavior_switch',
                                      '__' * (P('TOC') + 'FORCETOC' + 'NOTOC' +
                                              'NOEDITSECTION' + 'NOCC' +
                                              'NOINDEX') * '__') * #lexer.space)
lex:add_style('behavior_switch', lexer.STYLE_KEYWORD)

return lex
