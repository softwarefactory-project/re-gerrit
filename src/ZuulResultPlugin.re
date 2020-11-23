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

/*
   Apply change to a local gerrit with:

   parcel build src/ZuulResultPlugin.bs.js && \
     cp dist/ZuulResultPlugin.bs.js /var/gerrit/plugins/zuul.js && \
     ssh -p 29418 admin@localhost gerrit plugin reload zuul

   Enable remote plugin reload by adding to gerrit.config:
     [plugins]
         allowRemoteAdmin = true
 */

// Dom binding types
type table
and cell
and elem;

// Gerrit install binding
let install: (Gerrit.Change.t => unit) => unit = [%raw
  {|(showCallback) => {
    Gerrit.install(function (self) {
      self.on("showchange", showCallback);
      console.log("ZuulResultPlugin installed!")
    });
  }|}
];

// Dom manipulation bindings
let createTable: string => table = [%raw
  {|(name) => {
    let root = document.getElementById(
      "app").shadowRoot.getElementById(
      "app-element").shadowRoot.querySelector(
      "gr-change-view").shadowRoot.querySelector(
      ".commitContainer")
    let container = root.querySelector('#' + name);
    if (container === null) {
      container = document.createElement("table");
      root.appendChild(container);
    } else {
      container.innerHTML = "";
    }
    return container;
  }|}
];
let createCell: (option(string), elem) => cell = [%raw
  {|(style, elem) => {
    let cell = document.createElement("td");
    cell.style.cssText = "padding: 2px 4px 2px 6px; 2px solid #dcdcdc;" + style;
    cell.appendChild(elem);
    return cell;
  }|}
];
let addRow: (table, array(cell)) => unit = [%raw
  {|(table, cells) => {
     let row = document.createElement("tr");
     cells.forEach(cell => row.appendChild(cell));
     table.appendChild(row);
  }|}
];
let textCell: string => elem = [%raw "txt => document.createTextNode(txt)"];
let jobLink: (string, string) => elem = [%raw
  {|(name, url) => {
     let a = document.createElement("a");
     a.appendChild(textCell(name));
     a.href = url;
     return a;
  }|}
];
let jobResult: (option(string), string, string) => elem = [%raw
  {|(style, result, time) => {
     let span = document.createElement("span");
     let resSpan = document.createElement("span");
     if (style) {
       resSpan.style.color = style;
     };
     resSpan.appendChild(textCell(result));
     span.appendChild(resSpan);
     span.appendChild(textCell(" " + time));
     return span;
  }|}
];

// Helper function to create the cells
let headerCell = (txt: string): cell =>
  "background-color: #dcdcdc;"->Some->createCell(txt->textCell);
let bodyCell: elem => cell = None->createCell;
let addHeaderRow = (table: table, name: string, date: string): unit =>
  table->addRow([|name->headerCell, date->headerCell|]);
let addResultRow = (table, name, url, color, result, time): unit =>
  table->addRow([|
    name->jobLink(url)->bodyCell,
    color->jobResult(result, time)->bodyCell,
  |]);

// The zuul result plugin implementation:
let showBuild = (table: table, build: CI.Result.build): unit => {
  table->addResultRow(
    build.job,
    build.url,
    build.result->CI.Result.color,
    build.result,
    build.time,
  );
};
let showResults = (table: table, results: CI.Results.t): unit => {
  table->addHeaderRow(
    results.latests.name
    ++ " "
    ++ results.latests.pipeline
    ++ (
      results.count > 1
        ? " (" ++ string_of_int(results.count) ++ " rechecks)" : ""
    ),
    results.latests.date->Js.Date.toUTCString,
  );
  results.latests.builds->Belt.Array.map(table->showBuild)->ignore;
};
let showCallback = (change: Gerrit.Change.t): unit => {
  Js.log2("Zuul Result of change: ", change);
  change
  ->CI.Results.fromChange
  ->Belt.Array.map("zuulResult"->createTable->showResults)
  ->ignore;
};

showCallback->install;
