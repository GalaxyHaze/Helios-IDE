# Visual language

## Intent

Helios should feel like a focused native workbench: quiet chrome, high editor contrast, and strong but controlled status colors. The visual language should support long coding sessions without turning every state into a notification.

## Decisions

- Use a dark-first neutral canvas with blue-cyan accents for active navigation and focus.
- Keep semantic colors distinct: success is green, warning is amber, error is red, and informational progress is blue.
- Use one expressive display face for product headings and a readable monospace face for code and technical metadata.
- Prefer layered surfaces over heavy borders: canvas, panel, raised panel, and active surface.
- Use accent color for interaction state, not decoration.
- Reserve motion for state changes that need orientation: panel reveal, tab activation, and asynchronous task progress.

## Invariants

- Text and code remain readable when the accent color is removed.
- A user can identify the active editor, active panel, and current error without relying on motion.
- Theme changes preserve semantic meaning even when hue values change.
- Small screens collapse secondary panels before reducing editor readability.

## Future review questions

- Does a new color communicate a semantic state or merely add decoration?
- Does typography clarify hierarchy or compete with code?
- Does motion explain a state transition or delay a routine action?
