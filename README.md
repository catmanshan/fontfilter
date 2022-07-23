# Purpose
**`fontfilter` extends the capabilities of `fontconfig` by taking a different approach to font matching.**
Specifically, `fontfilter` provides an interface for specifying complex conditions, logically composing conditions (p and q, p or q, etc.), and filtering against a single condition or a priority-ranked list of conditions.

## Examples:
`fontfilter` could be used to find:
- monospace fonts
- non-proportional fonts
- fonts that support the character "あ"
- fonts whose families contain the word "Sans"
- fonts that are bold and italic
- fonts that are bold or italic, but not both
- fonts that are oblique or italic, but preferably italic
- fonts whose weights are between bold and heavy
