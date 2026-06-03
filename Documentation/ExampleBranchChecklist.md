# Example Branch Checklist

Use this checklist before publishing or sharing an example branch.

## Identity

- [ ] Branch name follows `example/<product>/<topic>` or `integration/<products>/<topic>`.
- [ ] Root `README.md` has a short "Current Example" summary for this branch.
- [ ] `Documentation/CurrentExample.md` explains the tutorial from the buyer perspective.
- [ ] The required Tiny Tool Development product or plugin is named clearly.
- [ ] Product documentation from the related `.uplugin` is linked when available.

## Setup

- [ ] Required plugins are enabled in `TinyToolDev_Examples.uproject`.
- [ ] Project Settings contain the correct branch-specific default map if the example uses one.
- [ ] Setup instructions mention all external requirements.
- [ ] Secrets, API keys, credentials, local paths, and machine-specific settings are not committed.
- [ ] Branch-specific config files include safe defaults.

## C++ Samples

- [ ] Public classes use clear Unreal reflection metadata for Blueprint users.
- [ ] Code comments explain the tutorial-relevant decisions, not obvious syntax.
- [ ] Header comments say what a beginner should learn from the class.
- [ ] Build dependencies are scoped to the modules actually needed by this branch.
- [ ] The project compiles after a clean editor restart.

## Blueprint Samples

- [ ] The entry Blueprint, widget, or map is named in the documentation.
- [ ] Important Blueprint variables have friendly display names and tooltips.
- [ ] The branch describes the main Blueprint graph flow in text or screenshots.
- [ ] The sample can be run without hunting through unrelated assets.

## Buyer Experience

- [ ] The first successful action takes less than five minutes.
- [ ] Expected output is described clearly.
- [ ] Common mistakes and troubleshooting notes are included.
- [ ] The sample explains how to adapt the pattern to a real project.
- [ ] The branch stays focused on one product feature or integration idea.

## Repository Hygiene

- [ ] `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, and `.vs` are not committed.
- [ ] Large assets are intentional and should use Git LFS in the remote repository.
- [ ] `git diff --check` passes.
- [ ] `.uproject` JSON validates.
- [ ] No unrelated changes from another example branch are mixed in.
