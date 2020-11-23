module Change = {
  type t = {
    project: string,
    branch: string,
    topic: option(string),
    status: string,
    messages: array(message),
  }
  and message = {
    id: string,
    date: string,
    message: string,
    _revision_number: int,
    author,
  }
  and author = {name: string};
};
