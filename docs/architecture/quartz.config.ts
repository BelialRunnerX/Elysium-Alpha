import { QuartzConfig } from "./quartz/cfg"
import { Plugin } from "./quartz/plugin"

const config: QuartzConfig = {
  configuration: {
    pageTitle: "Elysium",
    pageTitleSuffix: "",
    enableToc: true,
    enableLinkPreviews: true,
    enableRecentNotes: true,
    locale: "en-US",
    baseUrl: "https://belialrunnerx.github.io/elysium",
    ignorePatterns: ["node_modules"],
    defaultDateType: "created",
    theme: {
      fontOrigin: "googleFonts",
      cdnCaching: true,
      typography: {
        header: "Cinzel",
        body: "Inter",
        code: "IBM Plex Mono",
      },
      colors: {
        lightMode: {
          light: "#0a0c14",
          lightgray: "#1a1f2e",
          gray: "#4a5568",
          darkgray: "#a0aec0",
          dark: "#e2e8f0",
          secondary: "#10b981",      // Emerald
          tertiary: "#34d399",       // Light emerald
          highlight: "rgba(16, 185, 129, 0.15)",
          textHighlight: "#10b98133",
        },
        darkMode: {
          light: "#0a0c14",
          lightgray: "#1a1f2e",
          gray: "#4a5568",
          darkgray: "#a0aec0",
          dark: "#e2e8f0",
          secondary: "#10b981",      // Emerald
          tertiary: "#34d399",
          highlight: "rgba(16, 185, 129, 0.15)",
          textHighlight: "#10b98133",
        },
      },
    },
  },
  plugins: {
    transformers: [
      Plugin.FrontMatter(),
      Plugin.CreatedModifiedDate({
        priority: ["frontmatter", "git", "filesystem"],
      }),
      Plugin.SyntaxHighlighting({
        theme: {
          light: "github-dark",
          dark: "github-dark",
        },
        keepBackground: false,
      }),
      Plugin.ObsidianFlavoredMarkdown({ enableInHtmlEmbed: false }),
      Plugin.GitHubFlavoredMarkdown(),
      Plugin.TableOfContents(),
      Plugin.CrawlLinks({ markdownLinkResolution: "shortest" }),
      Plugin.Description(),
      Plugin.Latex({ renderEngine: "katex" }),
    ],
    filters: [Plugin.RemoveDrafts()],
    emitters: [
      Plugin.AliasRedirects(),
      Plugin.ComponentResources(),
      Plugin.ContentPage(),
      Plugin.FolderPage(),
      Plugin.TagPage(),
      Plugin.ContentIndex({
        enableSiteMap: true,
        enableRSS: true,
      }),
      Plugin.Assets(),
      Plugin.Static(),
      Plugin.NotFoundPage(),
    ],
  },
}

export default config
