let commentMessage =
  Gerrit.Change.{
    id: "test",
    date: "today",
    _revision_number: 1,
    author: {
      name: "Dev",
    },
    message: "LGTM",
  };

let fakeMessage =
  Gerrit.Change.{
    id: "test",
    date: "today",
    _revision_number: 42,
    author: {
      name: "Zuul CI",
    },
    message: "Patch Set 1: Verified+1

Build succeeded (check pipeline).

- zuul-build-image http://logs/f7490be/ : SUCCESS in 29m 22s (non-voting)
",
  };

let fakeChange = {
  Gerrit.Change.{
    project: "demo",
    branch: "main",
    topic: None,
    status: "NEW",
    messages: [|commentMessage, fakeMessage, fakeMessage|],
  };
};

let spec = [
  fakeMessage->CI.Zuul.fromMessage
  == CI.Result.{
       name: "Zuul CI",
       pipeline: "check",
       date: "today"->Js.Date.fromString,
       builds: [|
         {
           job: "zuul-build-image",
           url: "http://logs/f7490be/",
           time: "29m 22s (non-voting)",
           result: "SUCCESS",
         },
       |],
     }
     ->Some,
];

Node.Process.exit(spec->Belt.List.every(x => x) ? 0 : 1);
