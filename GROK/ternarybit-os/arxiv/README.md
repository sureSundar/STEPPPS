# arXiv Submission Pack

This directory helps you prepare an arXiv submission for the research paper.

## Contents
- `generate-tex.sh`: converts `RESEARCH_PAPER.md` to LaTeX using Pandoc.
- `metadata.txt`: template for arXiv metadata (title, abstract, categories).

## Steps
1) Generate LaTeX from the root of the repository or from here:
   ```bash
   bash arxiv/generate-tex.sh
   ```
   This creates `arxiv/RESEARCH_PAPER.tex`.

2) Verify the paper builds locally with `pdflatex` (recommended):
   ```bash
   cd arxiv
   pdflatex RESEARCH_PAPER.tex || true
   ```

3) Prepare arXiv submission:
   - Upload `RESEARCH_PAPER.tex` (and any figures if later added).
   - Choose categories: `cs.OS` (Operating Systems), `cs.AI` (Artificial Intelligence).
   - Use the Abstract from `RESEARCH_PAPER.md`.

4) After arXiv assigns an ID, update the README with the arXiv link and (optionally) keep Zenodo DOI as the canonical citation.
