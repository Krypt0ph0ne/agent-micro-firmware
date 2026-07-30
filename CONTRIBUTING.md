# Contributing

Thank you for improving Agent Micro.

1. Discuss hardware-affecting changes in an issue first.
2. Fork the repository and create a focused branch.
3. Run `make doctor`, `make test`, and `git diff --check`.
4. Never commit device UIDs, DataFlash/factory backups, private logs, secrets,
   generated binaries, or seller imagery.
5. Add or update documentation and tests for behavior changes.
6. Sign every commit for the Developer Certificate of Origin:

   ```bash
   git commit -s
   ```

   The sign-off certifies that you have the right to submit the contribution
   under this repository's licenses. Read the
   [Developer Certificate of Origin 1.1](https://developercertificate.org/).

Pull requests must pass CI and receive maintainer review. Contributions to
the firmware are accepted under CC BY-SA 3.0; contributions to files already
identified as MIT-licensed are accepted under MIT.
