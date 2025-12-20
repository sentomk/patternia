# Security Policy

## Supported Versions

Patternia is an actively developed header-only C++ library.
Security-related fixes are applied **only to the latest released minor version**.

Older releases are **not** maintained or backported.

| Version      | Supported |
|-------------|-----------|
| Latest 0.x  | ✔ Yes     |
| Older 0.x   | ✘ No      |

Users are strongly encouraged to upgrade to the latest release.

---

## Scope

Patternia is a **compile-time / header-only pattern matching library**.
It does not perform I/O, networking, deserialization, or process untrusted data
by itself.

As such, security issues are generally limited to:

- Undefined behavior triggered by valid user code
- Incorrect template instantiations leading to memory safety issues
- ODR / ABI violations caused by library implementation
- Compile-time denial-of-service via excessive template instantiation

Application-level security issues are **out of scope**.

---

## Reporting a Vulnerability

If you believe you have found a security issue in Patternia, please report it via:

- **GitHub Security Advisories**  
  https://github.com/sentomk/patternia/security/advisories

Please include:

- A minimal reproducible example
- Compiler and version
- Platform (OS / architecture)
- Patternia version

You can expect an initial response within **7 days**.

If the report is accepted, a fix will be prepared and released publicly.
If the report is declined, a brief explanation will be provided.

---

Thank you for helping improve the security and reliability of Patternia.
