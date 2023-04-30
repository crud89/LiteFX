**Describe the pull request**

Give a short introduction over what your pull request implements or fixes.

**Related issues**

Are there any issues that relate to this PR? Please link them.

**Checklist**

- [ ] If this PR is work in progress, open it as a [draft PR](https://github.blog/2019-02-14-introducing-draft-pull-requests/).
- [ ] If this PR fixes a bug, create a test that re-produces the bug.
- [ ] If this PR implements a feature, create tests accordingly.
- [ ] Before submitting the PR, make sure to update the `.docs/release-logs/` for the next version with a summary of the changes.
  - If the PR has to break source compatibility †, a new minor release should be targeted;
  - Otherwise target a new patch release.

† Source compatibility means that code that compiles against the current version, should also compile against the next version.