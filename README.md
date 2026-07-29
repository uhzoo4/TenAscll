# TENASCII site

A scroll-driven showcase page for the TENASCII/TENET project, built with
[Vite](https://vite.dev) and real [anime.js v4](https://animejs.com) effects
(`splitText`, `svg.createDrawable`, `svg.createMotionPath`, `onScroll`,
`createTimeline`).

## Where each file goes

Make a new folder (call it whatever you want, e.g. `tenascii-site`) and put
the files exactly like this — the paths matter, Vite expects this layout:

```
tenascii-site/            <- the folder you create
├── package.json          <- from the chat, goes at the root
├── index.html             <- from the chat, goes at the root
├── public/
│   └── img2art-preview.png   <- the screenshot used in the "showcase" section
└── src/
    ├── main.js            <- all the animation code
    └── style.css          <- all the styling / color tokens
```

`public/` and `src/` are plain folders you create yourself — nothing special
about them except the names, which Vite looks for by convention. If a file
lands in the wrong spot (e.g. `main.js` at the root instead of inside `src/`)
the `<script type="module" src="/src/main.js">` tag in `index.html` won't
find it and the page will load with no animations.

## Run it

From inside that folder:

```
npm install
npm run dev
```

That prints a local URL (usually `http://localhost:5173`) — open it in a
browser. `npm install` reads `package.json` and pulls in Vite and anime.js
automatically; you don't need to install anime.js separately.

To produce a deployable static build:

```
npm run build
```

Output lands in `dist/` — that folder is what you'd upload to any static
host (Netlify, Vercel, GitHub Pages, etc.). `npm run preview` serves that
build locally if you want to check it before deploying.

## Before you deploy

- `index.html` has a placeholder repo link (`href="#"` in the footer) —
  point it at your actual GitHub URL.
- Color tokens are the `:root` block at the top of `src/style.css` if you
  want to retune anything.
