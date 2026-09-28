"""
MkDocs hook for content that only belongs in specific versions of the website.

Sections between `<!-- only: latest -->` and `<!-- end-only -->` are only included when the matching version is built, and removed from all
other versions:

    <!-- only: latest -->
    !!! warning "Work in progress"

        This section documents a feature that is not part of a release yet.

    <!-- end-only -->

The condition accepts several versions, separated by commas (e.g. `only: latest, 0.5`), and can be negated with `not`, for example
`<!-- only: not latest -->` for content that appears in every version except `latest`.

The version being built is provided by mike (the environment variable `MIKE_DOCS_VERSION`). Local builds with `mkdocs serve` or
`mkdocs build` have no version, and behave like `latest`, so that work-in-progress notes are visible while writing.
"""

from __future__ import annotations

import os
import re

from mkdocs.structure.pages import Page

DEFAULT_VERSION = "latest"
BLOCK = re.compile(r"[ \t]*<!--\s*only:\s*(?P<condition>[^>]*?)\s*-->[ \t]*\r?\n?"
                   r"(?P<content>.*?)"
                   r"[ \t]*<!--\s*end-only\s*-->[ \t]*\r?\n?", re.DOTALL)


def current_version() -> str:
    return os.environ.get("MIKE_DOCS_VERSION") or DEFAULT_VERSION


def keep(condition: str, version: str) -> bool:
    negated = condition.lower().startswith("not ")
    versions = { v.strip() for v in (condition[4:] if negated else condition).split(",") if v.strip() }
    return (version not in versions) if negated else (version in versions)


def on_page_markdown(markdown: str, page: Page, config, files, **kwargs) -> str:
    version = current_version()
    return BLOCK.sub(lambda match: match.group("content") if keep(match.group("condition"), version) else "", markdown)