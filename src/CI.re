module Result = {
  type t = {
    name: string,
    pipeline: string,
    date: Js.Date.t,
    builds: array(build),
  }
  and build = {
    job: string,
    url: string,
    time: string,
    result: string,
  };
  let color = (result: string): option(string) =>
    switch (result) {
    | "SUCCESS" => "green"->Some
    | "SKIPPED" => None
    | _ => "red"->Some
    };
};

module Zuul = {
  let authorRe = [%re "/^(Zuul.*)/i"];
  let headerRe = [%re "/^Build \\w+ \\(([-\\w]+) pipeline\\)/"];
  let resultRe = [%re "/^- ([^ ]+) ([^ ]+) : ([^ ]+) in (.*)/"];
  let nullOpt = Js.Nullable.toOption;
  let firstMatch = (re, txt) =>
    re
    ->Js.Re.exec_(txt)
    ->Belt.Option.flatMap(res =>
        Js.Nullable.toOption(Js.Re.captures(res)[1])
      );
  // Extract build info from a message line
  let buildFromMessage = (line: string): option(Result.build) => {
    resultRe
    ->Js.Re.exec_(line)
    ->Belt.Option.flatMap(res =>
        switch (res->Js.Re.captures) {
        | [|_, job, url, result, time|] =>
          switch (job->nullOpt, url->nullOpt, result->nullOpt, time->nullOpt) {
          | (Some(job), Some(url), Some(result), Some(time)) =>
            Result.{job, url, result, time}->Some
          | _ => None
          }
        | _ => None
        }
      );
  };
  // Extract zuul build report from a message
  let fromMessage = (message: Gerrit.Change.message): option(Result.t) => {
    let lines =
      Js.Array.sliceFrom(2, "\n"->Js.String.split(message.message));
    let content = Js.Array.joinWith("\n", lines);
    authorRe
    ->firstMatch(message.author.name)
    ->Belt.Option.flatMap(name => {
        headerRe
        ->firstMatch(content)
        ->Belt.Option.flatMap(pipeline => {
            let builds =
              Js.Array.sliceFrom(2, lines)
              ->Belt.Array.keepMap(buildFromMessage);
            Result.{
              name,
              pipeline,
              builds,
              date: message.date->Js.Date.fromString,
            }
            ->Some;
          })
      });
  };
};
module Results = {
  type t = {
    count: int,
    latests: Result.t,
  };
  // Get latest patchset CI results
  let addResult = (history: list(t), current: Result.t): list(t) => {
    let rec go = (xs, replaced: bool, acc: list(t)) =>
      switch (xs) {
      | [] =>
        replaced ? acc : acc->Belt.List.add({count: 1, latests: current})
      | [x, ...xs] =>
        x.latests.pipeline == current.pipeline
          ? xs->go(
              true,
              acc->Belt.List.add({count: x.count + 1, latests: current}),
            )
          : xs->go(false, acc)
      };
    history->go(false, []);
  };
  let fromMessages = (messages: array(Gerrit.Change.message)): array(t) => {
    let rec go = (xs, ps, acc) => {
      switch (xs) {
      | [] => acc->Belt.List.toArray
      | [x, ...xs] =>
        switch (x->Zuul.fromMessage) {
        | Some(result) =>
          let history = x._revision_number == ps ? acc : [];
          xs->go(x._revision_number, history->addResult(result));
        | _ => xs->go(ps, acc)
        }
      };
    };
    messages->Belt.List.fromArray->go(1, []);
  };
  let fromChange = (change: Gerrit.Change.t): array(t) =>
    change.messages->fromMessages;
};
