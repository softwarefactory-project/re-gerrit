let commentMessage =
  Change.{
    id: "test",
    date: "today",
    _revision_number: 1,
    author: {
      name: "Dev",
    },
    message: "LGTM",
  };

let ciMessage =
  Change.{
    id: "test",
    date: "2020-11-24",
    _revision_number: 42,
    author: {
      name: "Zuul CI",
    },
    message: "Patch Set 1: Verified+1

Build succeeded (check pipeline).

- zuul-build-image http://logs/f7490be/ : SUCCESS in 29m 22s (non-voting)
",
  };

let ciResult =
  CI.Result.{
    name: "Zuul CI",
    pipeline: "check",
    date: "2020-11-24"->Js.Date.fromString,
    builds: [|
      {
        job: "zuul-build-image",
        url: "http://logs/f7490be/",
        time: "29m 22s (non-voting)",
        result: "SUCCESS",
      },
    |],
  };

let fakeChange = {
  Change.{
    project: "demo",
    branch: "main",
    topic: None,
    status: "NEW",
    messages: [|commentMessage, ciMessage, ciMessage|],
  };
};

let spec: list(bool) = [
  ciMessage->CI.Zuul.fromMessage == ciResult->Some,
  fakeChange->CI.Results.fromChange
  == [|CI.Results.{count: 2, latests: ciResult}|],
];

Node.Process.exit(spec->Belt.List.every(x => x) ? 0 : 1);
