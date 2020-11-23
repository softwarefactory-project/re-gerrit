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

// PolyGerrit plugin Javascript api
// https://gerrit-review.googlesource.com/Documentation/js-api.html

// Gerrit install binding
let install: (Change.t => unit) => unit = [%raw
  {|(showCallback) => {
    Gerrit.install(function (self) {
      self.on("showchange", showCallback);
      console.log("ZuulResultPlugin installed!")
    });
  }|}
];
