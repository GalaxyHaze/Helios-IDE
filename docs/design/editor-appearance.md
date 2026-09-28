# Editor appearance

## Intent

The editor surface should provide a quiet, high-contrast field for sustained
reading and editing. Visual emphasis must explain state without competing with
the document text.

## Decisions

- The document canvas is the strongest surface in the workspace; chrome uses
  quieter values from the same visual language.
- The gutter is visibly separate from the document but remains subordinate to
  the code.
- The current line uses a restrained full-width emphasis rather than a bright
  border or a moving marker.
- Selection is distinct from current-line emphasis and remains readable over
  syntax colors.
- Bracket matches use a compact, high-contrast decoration anchored to the
  matching characters.
- Diagnostic colors communicate severity through restrained background or
  underline treatments; they must not obscure source text.
- Theme changes preserve the same hierarchy across light and dark palettes.

## Invariants

- Text remains the highest-priority visual content.
- Diagnostic, bracket, selection, and current-line treatments must remain
  distinguishable when they overlap.
- Gutter changes must not alter document layout or horizontal text alignment.
- A theme change updates all editor surfaces consistently, including the
  gutter and transient decorations.

## Review questions

- Does a new color explain a state, or merely add decoration?
- Can a user identify the active line and selection without losing syntax
  contrast?
- Does the design remain legible at the smallest supported editor font size?
- If a new editor surface is added, does it belong to this aspect or to the
  application shell/tooling feedback aspect?
