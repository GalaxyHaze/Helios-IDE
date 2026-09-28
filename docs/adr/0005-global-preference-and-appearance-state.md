---
status: accepted
---

# Global preference and appearance state

Helios uses process-wide managers as the source of truth for preferences, themes, translations, and effective appearance. Preference setters persist changes immediately; appearance changes update the Qt application palette or font and notify interested widgets through Qt signals.

## Considered options

- Let each panel own its preference and appearance state: rejected because it would allow inconsistent themes, locales, and persisted values across the window.
- Defer all persistence until application shutdown: rejected because current settings are expected to survive individual changes and crashes.

## Consequences

- UI modules can observe one shared theme, locale, and preference state.
- A preference mutation may perform file I/O immediately, so persistence is part of the effective interface of the settings manager.
- Tests must isolate or redirect process-wide state when exercising settings and appearance behavior.
- Replacing global access with injected dependencies is a future architectural change, not an assumption of the current model.
