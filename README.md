# re-gerrit

Gerrit client

## Install

Add to your `package.json`:

```
npm install @softwarefactory-project/re-gerrit
```

Or using yarn:

```
yarn add @softwarefactory-project/re-gerrit
```

Add to your `bsconfig.json`:

```diff
"bs-dependencies": [
+  "@softwarefactory-project/re-gerrit"
]
```

## Install ZuulResult Gerrit plugin

Build from source:

```
$ yarn add parcel
$ yarn build
$ npx parcel build src/ZuulResultPlugin.re
```

Or extract the release from npm.

Then Copy the `dist/ZuulResultPlugin.bs.js` to `/var/gerrit/plugins/zuul.js` and restart.

## Contribute

Contributions are most welcome.
Get started by running:

```sh
git clone https://github.com/softwarefactory-project/re-gerrit
cd re-ansi
yarn install
yarn start
```

Then build and run tests with `yarn test`.

Make sure to read about [React][reason-react] and [Reason][rescript-lang] too.

## Changes

### 0.1.0

- Initial release

[reason-react]: https://reasonml.github.io/reason-react/docs/en/components
[rescript-lang]: https://rescript-lang.org/docs/manual/v8.0.0/overview
