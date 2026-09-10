# Getting a jar

Two routes. **GitHub Actions is the recommended one** — it builds on a clean
machine with a known JDK, so a failure is a real failure rather than something
odd about your setup.

---

## Route 1 — push to GitHub, let it build (recommended)

### 1. Unzip

Unzip `elysium-mod.zip` somewhere sensible. You should see `build.gradle`,
`gradlew`, `settings.gradle` and a `src/` folder at the top level. If instead
you see a single folder containing those, `cd` into it — the repository root is
wherever `build.gradle` lives.

### 2. Make a repository on GitHub

Go to <https://github.com/new>. Name it whatever you like — `elysium` is fine.
**Do not** tick "Add a README", "Add .gitignore" or "Choose a license"; the
project already has all three and an initialised repo makes the first push
awkward.

### 3. Push it

From a terminal in the folder from step 1:

```bash
git init
git add .
git commit -m "Elysium: NeoForge 1.21.1"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/YOUR-REPO.git
git push -u origin main
```

Replace the URL with the one GitHub showed you. If it asks for a password, use
a [personal access token](https://github.com/settings/tokens), not your account
password — GitHub stopped accepting passwords over HTTPS in 2021. The GitHub
CLI (`gh auth login`) handles this for you if you would rather not manage
tokens.

### 4. Collect the jar

The push starts a build automatically — the workflow at
`.github/workflows/build.yml` runs on every push.

1. Open your repository on GitHub and click the **Actions** tab.
2. Click the run at the top. It takes **10–20 minutes the first time**, because
   Gradle downloads and decompiles Minecraft before it can compile anything.
   Later runs are much quicker.
3. When it finishes, scroll to the bottom of the run's summary page to the
   **Artifacts** section and download **`elysium-jar`**.
4. Unzip that download. Inside is `elysium-1.0.0.jar` — that is the mod.

If the build fails, the workflow attaches **`build-reports`** in the same place
instead. Download it and send me the contents; it contains the actual compiler
output.

### 5. Install it

Put `elysium-1.0.0.jar` into your instance's `mods` folder, alongside
[NeoForge 21.1.248](https://neoforged.net/) for Minecraft 1.21.1. No other mods
are required — the Apotheosis, Silent Gear and Legendary Tooltips hooks all
check whether those mods are present and do nothing when they are not.

---

## Route 2 — build it locally

You need **JDK 21**. Not 17, not 22 — NeoForge for 1.21.1 targets 21
specifically. [Temurin 21](https://adoptium.net/temurin/releases/?version=21)
is the usual choice. Check what you have with `java -version`.

```bash
# macOS / Linux
chmod +x ./gradlew
./gradlew build

# Windows
gradlew.bat build
```

The jar lands at `build/libs/elysium-1.0.0.jar`.

The first run downloads Minecraft, decompiles it and remaps it, which takes
**several minutes and a few GB of disk**. It looks like it has hung around
"executing tasks" — it has not. Subsequent builds take seconds.

### Running it in a dev environment

```bash
./gradlew runClient     # a client with the mod loaded
./gradlew runServer     # a dedicated server, to test the networking
./gradlew runData       # regenerates data-pack JSON
```

`runServer` is worth doing at least once: the character system is the first
part of this mod with client/server networking in it, and a dedicated server is
the only place a client/server split can actually be proven.

---

## If something goes wrong

**"error: invalid source release 21"** or similar — Gradle is using the wrong
JDK. `./gradlew -version` prints the JVM it picked. Set `JAVA_HOME` to a JDK 21
installation.

**"Could not resolve net.neoforged:neoforge"** — no network, a proxy in the
way, or an outage at <https://maven.neoforged.net>. Nothing in the project can
work around it.

**"bad interpreter" on the CI runner** — `gradlew` was committed with Windows
line endings. `.gitattributes` pins it to LF, so this should not happen; if it
does, run `git add --renormalize . && git commit` and push again.

**Out of memory during decompilation** — raise the heap in
`gradle.properties`. It is already at `-Xmx2G`, up from the MDK default of 1G;
try 3G or 4G.

---

## What is in here besides the mod

- `README.md` — every system in the mod and how it works.
- `FIXES.md` — everything that was broken in the original and what it is now.
- `TEXTURES.md` — the art direction and the rules the sprites follow.
- `validate.py` — checks every model, texture, recipe, loot table, lang key and
  balance invariant. Run it with `python3 validate.py` from this folder after
  any change to the resources; it exits non-zero and says what is wrong.
- `tools/textures/` — the generator that produces every sprite in the mod.
  `python3 tools/textures/build.py` regenerates the lot. Needs Pillow.
