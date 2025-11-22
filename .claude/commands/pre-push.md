# Pre-Push Code Review

Perform a comprehensive senior-level code review of all unpushed changes before pushing to remote.

## Analysis Process

1. **Detect Unpushed Changes**
   - Run `git log @{u}..HEAD --oneline` to check for unpushed commits
   - If no upstream branch exists, run `git log origin/main..HEAD --oneline`
   - Run `git diff @{u}..HEAD` (or `git diff origin/main..HEAD`) to get all changes
   - If no unpushed commits exist, inform the user and exit

2. **Analyze Changed Files**
   - Extract all modified files from the diff
   - Read each modified file completely
   - Perform deep analysis on changed sections with surrounding context

3. **Code Review Categories**

   Categorize all findings by **Severity** and **Category**:

   **Severity Levels:**
   - **Critical**: Security vulnerabilities, data loss risks, crashes
   - **High**: Logic errors, race conditions, resource leaks
   - **Medium**: Code smells, maintainability issues, minor bugs
   - **Low**: Style inconsistencies, minor optimizations
   - **Info**: Suggestions, best practices, documentation

   **Categories:**
   - **Security**: Vulnerabilities, unsafe operations, credential exposure
   - **Bugs**: Logic errors, incorrect behavior, edge cases
   - **Quality**: Code smells, maintainability, readability, patterns
   - **Performance**: Inefficiencies, bottlenecks, resource usage
   - **Style**: Formatting, naming conventions, code organization

4. **Analysis Focus Areas**
   - Security vulnerabilities (injection, XSS, authentication, authorization)
   - Null/undefined handling and type safety
   - Error handling and edge cases
   - Resource management (memory leaks, file handles, connections)
   - Race conditions and concurrency issues
   - Performance bottlenecks (N+1 queries, inefficient loops)
   - Code duplication and maintainability
   - Naming conventions and code clarity
   - Missing error handling or logging
   - Type safety and validation
   - Documentation and comments quality

5. **Output Format**

   For each finding, provide:

   ```
   ### [SEVERITY] Category: Brief Description

   **File:** `path/to/file.py:line_number`

   **Issue:**
   Clear explanation of the problem and why it matters.

   **Current Code:**
   ```language
   // Show the problematic code with line numbers
   ```

   **Proposed Fix:**
   ```language
   // Show the corrected code
   ```

   **Impact:** Explain the impact if not fixed.
   ```

6. **Summary Report**

   After analyzing all changes, provide:

   ```
   ## Code Review Summary

   ### Statistics
   - Total files changed: X
   - Total findings: Y
   - Critical: X | High: X | Medium: X | Low: X | Info: X

   ### Findings by Category
   - Security: X
   - Bugs: X
   - Quality: X
   - Performance: X
   - Style: X

   ### Files Analyzed
   - file1.py (3 findings)
   - file2.py (1 finding)
   ```

7. **User Decision**

   After presenting all findings, ask:

   ```
   Would you like me to implement these changes automatically?

   Type 'yes' to apply all proposed fixes
   Type 'no' to abort and review manually
   ```

8. **Implementation (if "yes")**
   - Use the TodoWrite tool to create tasks for each file to be modified
   - Apply changes using the Edit tool
   - Show each change as it's applied
   - Mark todos as completed after each file
   - Run any relevant tests if applicable
   - Provide final summary of applied changes

9. **Abort (if "no")**
   - Acknowledge the user's decision
   - Remind them to review the findings manually
   - Exit gracefully

## Important Notes

- Be thorough but practical - focus on real issues, not nitpicking
- Consider the project's existing patterns and style
- Prioritize security and correctness over style
- Provide actionable feedback with clear examples
- If no issues are found, congratulate the user on clean code
- Never apply changes without explicit user confirmation
- Use markdown file links for all file references: [filename.py:42](path/to/filename.py#L42)