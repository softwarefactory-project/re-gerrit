// Copyright (c) 2020 Red Hat
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.

// CI result
open Belt;

module CIResult = {
  // A single CI result
  type t = {
    name: string,
    pipeline: option(string),
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

  // Regular expression to match CI result comments
  let authorRe = [%re "/^(.* CI|Zuul)/"];
  let resultRe = [%re "/^- ([^ ]+) ([^ ]+) : ([^ ]+) in (.*)/"];

  // Helper functions
  let nullOpt = Js.Nullable.toOption;
  let firstMatch = (re: Js.Re.t, txt: string): option(string) =>
    re
    ->Js.Re.exec_(txt)
    ->Option.flatMap(res => Js.Re.captures(res)[1])
    ->Option.flatMap(nullOpt);

  // Extract build info from a message line
  let buildFromMessage = (line: string): option(build) => {
    resultRe
    ->Js.Re.exec_(line)
    ->Option.flatMap(res =>
        switch (res->Js.Re.captures) {
        | [|_, job, url, result, time|] =>
          switch (job->nullOpt, url->nullOpt, result->nullOpt, time->nullOpt) {
          | (Some(job), Some(url), Some(result), Some(time)) =>
            {job, url, result, time}->Some
          | _ => None
          }
        | _ => None
        }
      );
  };
  // Extract zuul build report from a message
  let fromMessage =
      (pipelineMatch, message: Gerrit.Change.message): option(t) => {
    let lines =
      Js.Array.sliceFrom(2, "\n"->Js.String.split(message.message));
    let content = Js.Array.joinWith("\n", lines);
    authorRe
    ->firstMatch(message.author.name)
    ->Option.flatMap(name => {
        pipelineMatch(content)
        ->Option.flatMap(pipeline =>
            {
              name,
              pipeline,
              builds:
                Js.Array.sliceFrom(2, lines)
                ->Array.keepMap(buildFromMessage),
              date: message.date->Js.Date.fromString,
            }
            ->Some
          )
      });
  };
};

module Zuul = {
  // Regular expression to match zuul result comments
  let headerRe = [%re "/^Build \\w+ \\(([-\\w]+) pipeline\\)\\./"];
  let fromMessage =
    CIResult.fromMessage(content =>
      headerRe
      ->CIResult.firstMatch(content)
      ->Option.flatMap(v => v->Some->Some)
    );
};

module Jenkins = {
  // Regular expression to match zuul result comments
  let headerRe = [%re "/^Build \\w+\\./"];
  //  let headerRe = [%re "/^Build \\w+\\.$/"];
  let fromMessage =
    CIResult.fromMessage(content =>
      headerRe->Js.Re.exec_(content)->Option.flatMap(_ => None->Some)
    );
};

module Results = {
  // A list of results with recheck count
  type t = {
    count: int,
    latests: CIResult.t,
  };

  // Get latest patchset CI results
  let addResult = (history: list(t), current: CIResult.t): list(t) => {
    let rec go = (xs, replaced: bool, acc: list(t)) =>
      switch (xs) {
      | [] =>
        // if result was not replaced, add it to the accumulator
        replaced ? acc : acc->List.add({count: 1, latests: current})
      | [x, ...xs] =>
        // if previous result exists, replace and increase the recheck count
        x.latests.name == current.name
        && x.latests.pipeline == current.pipeline
          ? xs->go(
              true,
              acc->List.add({count: x.count + 1, latests: current}),
            )
          : xs->go(replaced, acc->List.add(x))
      };
    history->go(false, [])->List.reverse;
  };
  let fromMessages = (messages: array(Gerrit.Change.message)): array(t) => {
    let rec go = (xs, ps, acc) => {
      switch (xs) {
      | [] => acc->List.toArray
      | [x, ...xs] =>
        switch (x->Zuul.fromMessage, x->Jenkins.fromMessage) {
        | (Some(result), _)
        | (_, Some(result)) =>
          // drop history if current patchset is more recent
          let history = x._revision_number == ps ? acc : [];
          xs->go(x._revision_number, history->addResult(result));
        | _ => xs->go(ps, acc)
        }
      };
    };
    messages->List.fromArray->go(1, []);
  };
  let fromChange = (change: Gerrit.Change.t): array(t) =>
    change.messages->fromMessages;
};
