**Describe the pull request**

Give a short introduction over what your pull request implements or fixes.

**Related issues**

Are there any issues that relate to this PR? Please link them.

**Checklist**

- [ ] Run static code analysis and fix potential issues it reports.
  - You can do this by building targets `windows-clangcl-x64-release` and `windows-clangcl-x86-release`.
  - Avoid using NOLINT comments; if unavoidable (e.g. for false-positives, etc.), please leave a comment within the PR body to explain it.
- [ ] If this PR is work in progress, open it as a [draft PR](https://github.blog/2019-02-14-introducing-draft-pull-requests/).
- [ ] If required, update the `.natvis` file(s) of the altered projects to reflect your changes.
- [ ] Before submitting the PR, make sure to update the `.docs/release-logs/` for the next version with a summary of the changes.
  - Consider creating a draft PR to acquire it's URL.
  - If the PR addresses a bug that can be fixed without breaking API compatibility†, leave a comment so that we can target the proper branch.

† API compatibility means that code that compiles against the current version, should also compile against the next version.