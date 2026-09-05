# Markdown to HTML Converter

A small C++ command-line tool that parses a Markdown file into a tree of nodes and renders it back out as HTML. Built as a learning project — a hand-rolled recursive parser rather than a wrapper around an existing Markdown library.
****

## Features
* Headings, # through ###### (h1–h6)
* Paragraphs
* Bold (**text** / __text__) and italic (*text* / _text_)
* Inline code (`code`)
* Fenced code blocks, delimited by matching runs of backticks or tildes
* Horizontal rules
* Links, [text](url "optional title")
* Images, ![alt](url "optional title")
* Unordered lists (*, +, -)
* Ordered lists (1., 2., ...)
* Recursive inline parsing, so formatting can nest (e.g. bold inside a link, code inside a list item)
* Basic HTML escaping of &, <, and > in source text


## How it works

The parser reads the source file line by line, building each line into a TreeNode (recursing into the line's contents to resolve inline formatting and links). Block-level constructs that span multiple lines — lists and fenced code blocks — pull additional lines from the file stream as needed. Once the whole document has been parsed into a tree, a second pass walks the tree and writes out the corresponding HTML tags.

### Where this diverges from the CommonMark spec

This isn't a spec-compliant Markdown parser, and a few choices depart from CommonMark on purpose or as a simplification:

Horizontal rules only recognise a line of exactly four asterisks (****), rather than the full CommonMark set (***, ---, ___, 3 or more characters, optionally spaced out).

A line of bare # characters with nothing following (no space, no text) is treated as a plain paragraph rather than CommonMark's "empty heading."
Consecutive non-blank lines are not merged into a single paragraph — each line becomes its own block-level element rather than being joined the way CommonMark joins paragraph text.

Lists aren't indentation-aware: consecutive lines sharing a list marker become siblings in a flat list rather than being nested by indentation level.
_ and * are treated identically for emphasis. CommonMark disallows _..._ from triggering emphasis mid-word (so snake_case_word should stay literal), but allows * to. This parser doesn't distinguish, so snake_case_word gets partially italicized.

Ordered lists only accept '.' as the delimiter, not ')', and the list's starting number is never preserved — <ol> never gets a start attribute even if the source starts at, say, 5.

## Not currently supported
* Blockquotes (>)
* Setext-style headings (underlined with === / ---)
* Tables
* Backslash escapes for literal Markdown characters
* Hard line breaks (trailing double-space)

## Requirements
A C++11 (or later) compiler. Developed and tested with C++ 17.

## Project structure
Currently a single-file project (main.cpp) — parser, tree structure, and HTML renderer all live together for simplicity.

## Contributing
This is primarily a personal/learning project, so some bugs and missing functionality is expected, but issues and pull requests are welcome if you spot a bug or want to extend the supported syntax.