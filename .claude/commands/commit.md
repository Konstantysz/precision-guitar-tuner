# Description

Analyze current git changes and create logical, atomic commits following best practices.

# Instructions

You are tasked with analyzing the current git changes and creating well-structured, atomic commits. Follow these steps:

1. **Analyze the current state:**
   - Run `git status` to see all changes
   - Run `git diff` to see unstaged changes
   - Run `git diff --staged` to see staged changes
   - Review recent commit history with `git log --oneline -10` to understand the project's commit style

2. **Determine logical grouping:**
   - Group changes by logical units (feature, bugfix, refactor, docs, tests, etc.)
   - Ensure each commit is atomic - it should represent one logical change
   - Consider file types and their relationships
   - Separate unrelated changes into different commits
   - Keep related changes together (e.g., implementation + tests, code + docs)

3. **Create commits:**
   - For each logical group, stage the relevant files using `git add`
   - Create a commit with a clear, concise single-line message
   - Do not use any type of prefix (e.g., `feat:`, `fix:`, `docs:`, `test:`)
   - **IMPORTANT**: Use single-line commit messages ONLY - no multi-line messages
   - **IMPORTANT**: Do NOT include any mention of AI, Claude, or automated tools in the commit message
   - Focus the message on WHAT changed and WHY (if not obvious from the what)
   - Use imperative mood (e.g., "Add feature" not "Added feature")

4. **Pre-commit hook handling:**
   - After each commit attempt, check if pre-commit hooks failed or modified files
   - If the commit succeeded but pre-commit hooks modified files:
     - Check authorship with `git log -1 --format='%an %ae'` to verify it's safe to amend
     - Check if not pushed with `git status` to ensure the branch is ahead of remote
     - If both checks pass, stage the hook modifications with `git add -u`
     - Amend the commit to include the pre-commit fixes: `git commit --amend --no-edit`
   - If the commit failed due to pre-commit hooks:
     - The hooks may have made automatic fixes - check with `git status` and `git diff`
     - If hooks made fixes, stage them with `git add -u` and retry the commit
     - If hooks reported errors without fixes, report the error to the user
     - Only retry once per commit to avoid infinite loops
   - **IMPORTANT**: Never amend commits from other developers - only amend if you just created it

5. **Verification:**
   - After creating all commits, run `git log --oneline -n <number>` to show the created commits
   - Run `git status` to confirm all intended changes are committed

6. **Report:**
   - Summarize what commits were created and why you grouped changes that way
   - Note any uncommitted changes that remain (if intentional)
   - Report any pre-commit hook fixes that were automatically applied

**Example commit messages:**
- `Add user authentication with JWT tokens`
- `Fix memory leak in data processing pipeline`
- `Refactor database connection pooling`
- `Update API documentation for v2 endpoints`
- `Add unit tests for OCR pipeline`

Begin the analysis now.
