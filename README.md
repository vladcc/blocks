# blocks
grep for nested data

## What is this and what does it do?

blocks is a generic fetcher for blocks of code/data. The way grep prints lines
which match certain criteria, blocks prints whole, well, blocks. It looks for
blocks starting with a given name and delimited by given open/close character
sequences. It can understand single line comments, multi line comments, and
strings, if applicable to the parsed language.

This makes it capable of doing plain text "queries" from the command line on
structured data. E.g. the equivalent of "out of these xml files fetch me the
tags which include value X", or "print me all functions from these files which
mention variable Y".

Here's a quick, albeit contrived, example:

Let's say we want to do some quick and dirty work with the following json file
(taken from https://json.org/example.html)
```
$ cat example.json
{
  "widget": {
    "debug": "on",
    "window": {
      "title": "Sample Konfabulator Widget",
      "name": "main_window",
      "width": 500,
      "height": 500
    },
    "image": {
      "src": "Images/Sun.png",
      "name": "sun1",
      "hOffset": 250,
      "vOffset": 250,
      "alignment": "center"
    },
    "text": {
      "data": "Click Here",
      "size": 36,
      "style": "bold",
      "name": "text1",
      "hOffset": 250,
      "vOffset": 100,
      "alignment": "center",
      "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
  }
}
```

Fetch only window, image, text:
```
$ ./blocks +r -n 'image|text|window' example.json
    "window": {
      "title": "Sample Konfabulator Widget",
      "name": "main_window",
      "width": 500,
      "height": 500
    },
    "image": {
      "src": "Images/Sun.png",
      "name": "sun1",
      "hOffset": 250,
      "vOffset": 250,
      "alignment": "center"
    },
    "text": {
      "data": "Click Here",
      "size": 36,
      "style": "bold",
      "name": "text1",
      "hOffset": 250,
      "vOffset": 100,
      "alignment": "center",
      "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
```

Out of these print the ones which have an hOffset property:
```
$ ./blocks +r -n 'image|text|window' example.json -m hOffset
    "image": {
      "src": "Images/Sun.png",
      "name": "sun1",
      "hOffset": 250,
      "vOffset": 250,
      "alignment": "center"
    },
    "text": {
      "data": "Click Here",
      "size": 36,
      "style": "bold",
      "name": "text1",
      "hOffset": 250,
      "vOffset": 100,
      "alignment": "center",
      "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
```

Now fetch the ones with hOffset and without src
```
$ ./blocks +r -n 'image|text|window' example.json -m hOffset -M src
    "text": {
      "data": "Click Here",
      "size": 36,
      "style": "bold",
      "name": "text1",
      "hOffset": 250,
      "vOffset": 100,
      "alignment": "center",
      "onMouseUp": "sun1.opacity = (sun1.opacity / 100) * 90;"
    }
```

Use with awk to turn image and window into greppable one-liners
```
$ ./blocks +r -n 'image|window' example.json -E@END | awk -vRS='@END' '{gsub("[[:space:]]+", " "); print}'
 "window": { "title": "Sample Konfabulator Widget", "name": "main_window", "width": 500, "height": 500 },
 "image": { "src": "Images/Sun.png", "name": "sun1", "hOffset": 250, "vOffset": 250, "alignment": "center" },

```

The same can be done with the xml representation of the same data
```
$ cat example.xml
<widget>
    <debug>on</debug>
    <window title="Sample Konfabulator Widget">
        <name>main_window</name>
        <width>500</width>
        <height>500</height>
    </window>
    <image src="Images/Sun.png" name="sun1">
        <hOffset>250</hOffset>
        <vOffset>250</vOffset>
        <alignment>center</alignment>
    </image>
    <text data="Click Here" size="36" style="bold">
        <name>text1</name>
        <hOffset>250</hOffset>
        <vOffset>100</vOffset>
        <alignment>center</alignment>
        <onMouseUp>
            sun1.opacity = (sun1.opacity / 100) * 90;
        </onMouseUp>
    </text>
</widget>
```
```
$ ./blocks --lang=xml -n 'image|window|text' example.xml
    <window title="Sample Konfabulator Widget">
        <name>main_window</name>
        <width>500</width>
        <height>500</height>
    </window>
    <image src="Images/Sun.png" name="sun1">
        <hOffset>250</hOffset>
        <vOffset>250</vOffset>
        <alignment>center</alignment>
    </image>
    <text data="Click Here" size="36" style="bold">
        <name>text1</name>
        <hOffset>250</hOffset>
        <vOffset>100</vOffset>
        <alignment>center</alignment>
        <onMouseUp>
            sun1.opacity = (sun1.opacity / 100) * 90;
        </onMouseUp>
    </text>
```
```
$ ./blocks --lang=xml -n 'image|window|text' -m hOffset -M src example.xml
    <text data="Click Here" size="36" style="bold">
        <name>text1</name>
        <hOffset>250</hOffset>
        <vOffset>100</vOffset>
        <alignment>center</alignment>
        <onMouseUp>
            sun1.opacity = (sun1.opacity / 100) * 90;
        </onMouseUp>
    </text>
```


## How does it work?
On a high level, it's glorified parenthesis matching. In more detail, it defines
a block as something with a name, an opening sequence, and a closing sequence.
The defaults for the three are ```{ { }``` (the name is the same as the opening
sequence), respectively. It looks for the block name closest to the block start
and fetches everything between the opening and closing sequences for that block.

It can match fixed strings and regular expressions. As mentioned in the
beginning, it can ignore comments and strings of a target laguage. It can
perform logic on the blocks it finds (e.g. print only the ones which don't have
the pattern x somewhere inside). It can mark blocks starts and ends for further
processing with e.g. awk.

Files can be given on the command line, as a list from a file, or blocks can
look for them itself, recursivelly or not, using regex to include/exclude file
names.

When matching patterns inside a block, the ```-m -M``` options can appear
multiple times. If they do, all ```-m``` must match the block together, all
```-M``` must not match the block together. The logic between the two can be
controlled to either match all ```-m``` ```and``` don't match all ```-M```, or
match all ```-m``` ```or``` don't match all ```-M```, i.e. the following:

```
if ((match(m_1) && match(m_2) ...) && (dont_match(M_1) && dont_match(M_2) ...))
	print_block()
```

```
if ((match(m_1) && match(m_2) ...) || (dont_match(M_1) && dont_match(M_2) ...))
	print_block()
```

Logic of the type "print the block only if this pattern or that pattern
matches/doesn't match" is handled with regex, e.g.
```-m 'foo|bar' -M 'zig|zag'```
will print only blocks which match foo or bar and don't match zig or zag.

Each of blocks' options can be turned on and off by itself. This can get
cumbersome so presets for the more common languages are offered with a single
option, e.g.

No language:
```
$ ./blocks -D
default block name: default block start
default block start: '{'
default block end: '}'
lang: none
block name: '{' type: string case: A
block start: '{' type: string case: A
block end: '}' type: string case: A
line comment: '' type: none case: none
block comment begin: '' type: none case: none
block comment terminate: '' type: none case: none
string rx: '' type: none case: none
files include: '' type: none case: none
files exclude: '' type: none case: none
files dir: none
recurse: no
no strings: off
match: '' type: none case: none
don't match: '' type: none case: none
match/don't match logic: none
```

C:
```
$ ./blocks --lang C -D
default block name: default block start
default block start: '{'
default block end: '}'
lang: c
block name: '{' type: string case: A
block start: '{' type: string case: A
block end: '}' type: string case: A
line comment: '//' type: string case: A
block comment begin: '/*' type: string case: A
block comment terminate: '*/' type: string case: A
string rx: '"([^\\"]|[\\].)*"' type: regex case: A
files include: '' type: none case: none
files exclude: '' type: none case: none
files dir: none
recurse: no
no strings: on
match: '' type: none case: none
don't match: '' type: none case: none
match/don't match logic: none
```

XML:
```
$ ./blocks --lang xml -D
default block name: default block start
default block start: '{'
default block end: '}'
lang: xml
block name: '<([_:a-zA-Z][-._:a-zA-Z0-9]*)' type: regex case: A
block start: '<([_:a-zA-Z][-._:a-zA-Z0-9]*)' type: regex case: A
block end: '</([_:a-zA-Z][-._:a-zA-Z0-9]*)' type: regex case: A
line comment: '<.*/>' type: regex case: A
block comment begin: '<!--' type: string case: A
block comment terminate: '-->' type: string case: A
string rx: '"[^"]*"|'[^']*'' type: regex case: A
files include: '' type: none case: none
files exclude: '' type: none case: none
files dir: none
recurse: no
no strings: on
match: '' type: none case: none
don't match: '' type: none case: none
match/don't match logic: none
```

For a full list of options see the help message.

## How to build?
```
make release && make test
```
