# Security Policy

The maintainers of **MeteoScope Engine** take security seriously. We appreciate your efforts to responsibly disclose any
vulnerabilities you may discover.

---

## Supported Versions

Only the latest release branch and current `main` branch receive security patches.

| Version | Supported          | Security Status |
|---------|--------------------|-----------------|
| 1.0.x   | :white_check_mark: | Supported       |
| < 1.0   | :x:                | End-of-Life     |

---

## Reporting a Vulnerability

If you discover a security vulnerability, please **DO NOT** open a public issue. Publicly disclosing a vulnerability can
endanger the community and any systems running this software.

Instead, please report security issues confidentially using one of the following methods:

1. **GitHub Security Advisory (Recommended):**
   Navigate to the repository's **Security** tab and select **Advisories** to submit a private vulnerability report. *(
   Note: This requires Private Vulnerability Reporting enabled in repository settings).*

2. **Direct Contact via GitHub:**
   Reach out directly to the maintainer via GitHub profile (**[@TheRealMiroslav](https://github.com/TheRealMiroslav)**).

### Information to Include in Your Report

To help us triage and resolve the issue quickly, please provide:

- A clear description of the vulnerability and its potential impact.
- Exact steps or minimal reproducible proof-of-concept (PoC) code/dataset triggering the vulnerability (e.g., malformed
  CSV payload causing memory corruption or buffer overrun).
- Affected environment (OS, compiler, CPU architecture, commit hash or release tag).
- Any suggested mitigations or patches, if known.

### Response Timeline

- **Acknowledgment:** Within **48 hours** of receipt.
- **Assessment & Triage:** Within **5 business days**, including an estimated fix timeline.
- **Coordinated Disclosure:** We will work with you to test the fix and agree upon a coordinated disclosure date and CVE
  assignment if applicable.

Thank you for helping keep MeteoScope Engine secure!
