# Production source bootstrap

The GitHub-side importer is ready. Upload the provided file named `production-source.tar.xz` into this `.bootstrap/` directory.

Expected SHA-256:

`39513de33512070237eeee88d47917b2b95747f9fb01d0d1e1432bf8e14fcac7`

The upload itself triggers `.github/workflows/bootstrap-import.yml`. The importer verifies the checksum, reconstructs the prepared production tree, expands the losslessly templated late-generation modules, verifies exactly 1,588 `src` + 22 `tests` + 2 `tools` C++ translation units (1,612 total), commits the expanded source to `main`, removes the bootstrap payload/workflow, and dispatches `windows-build.yml`.
