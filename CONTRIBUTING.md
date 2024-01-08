# Contributing to D1GT (Diablo 1 Graphics Tool)

Whenever you want to contribute to the project, whether it's by editing a simple file and fixing a typo, or implementing a huge feature which will add something new to the application - be sure that the change you are making is in a direction that the project is heading. You can make sure by either opening an issue and discussing it here first, or by asking on our discord sever, on an appropriate channel, like `#development`.

## Communication

Join our Discord server: [Diasurgical Discord](https://discord.gg/devilutionx)

## Creating an issue - guidelines

Issues are mostly created during the following occassions:

* There is a weird/undefined behavior of the app, and you're not sure if it's a bug or something that is expected to happen
* There is an obvious bug that for example causes the app to crash
* You're not sure if the implementation you want to make aligns with the project's direction - here you can create an issue to propose your implementation change and see if community approves it

For questions regarding your implementation change, you can always ask on our Discord, on `#development` channel.

Please try to do these when creating an issue:

* Try to prepend your issue's title with the category/component that the issue is related to, for example, if your change is mostly related to LevelCelView, you'd name your issue as follows: `LevelCelView: Modified <...>`. If you're not sure, you can drop the tag, and if the issue is most probably related to every, or many components of the app, you can use `Everywhere:` tag.
* Create only one issue per bug/implementation request
* Thoroughly describe the bug that you witnessed, if possible also add screenshots - there is no general pattern or template on how the issue should look like, so try to give the next person who will take care of it as many hints as possible
* Include your platform name that you've witnessed the bug on, and libraries versions that you're using on your system
* Please refrain from creating issues related to build problems or other support requests. If CI builds the application without any issues during a pull request, the problem is likely on your side. Instead, seek assistance on the #request-support channel on our Discord or initiate a proper Q&A discussion on the [Discussions](https://github.com/diasurgical/d1-graphics-tool/discussions) tab on GitHub.

## Creating a PR - guidelines

Those guidelines won't be necessarily as strictly executed as they should be, but it should be your responsibility to mark all the checks. If you're a future maintainer, or a future frequent contributor, you'd not want to debug a change that's not as descriptive or readable as it should be, at the same time not knowing if it introduced something important or not. So it's better for everyone to stick to the guidelines and always check them before opening a new Pull Request.

**Please stick to the following rules:**
* When opening a pull request, prepend the title with the category/component that the PR is related to, for example if your PR is mostly changing LevelCelView component, do as follows: `LevelCelView: Modified <...>`. If you're not sure, you can drop the tag, and if the PR is most probably related to every, or many components of the app, you can use `Everywhere:` tag.
* Split your changes to separate, small commits. If one commit changes resolution of the window, there shouldn't be an implementation of a button that adds rectangles to a view in the same commit.
* Write PR title and all commit messages in imperative mood, for example, don't do this: ("LevelCelView: Changed size of the window"), rather do this: ("LevelCelView: Change size of the window).
* Write everything in proper English, with proper punctuation
* Check spelling of your code and commit messages, and PR title
* Do not use slang inside the code or commit messages/PR title
* If you're adding a method to a class, it's encouraged to add documentation. By default we use Doxygen to generate documentation for our code. For examples how to describe your newly added methods you can check `config.cpp` file. Documentation should also be included in separate commits.
* If a change happened to a line that has been introduced in commit X, do not create a new commit Y, amend your changes to commit X and then push with force.
* Always make sure you're rebased on master to avoid conflicts with other contributors
* Try to wrap your commit messages on 72 character in a single line
* Right now we have a basic formatting check in CI, so to make sure you're in line with it, you can run `clang-format` on your files with `.clang-format` file in `source/` directory of the project.
* Do not resolve comments by yourself, if your change satisfies reviewer's request - they will resolve their comments, just let them know in an answer that the request has been satisfied.

## Behavior etiquette

We're all working on this project in our free time, many of us are frustrated after work or after a day of problems. We try to keep it purely technical, since everyone is from a different country, culture and everyone has a set of different beliefs. But to maintain things in a clear, transparent way we hold ourselves and respect a set of rules, so it would be good that everyone who joins the project, or is a frequent contributor would stick to those:

* Do not advertise your personal politics or religious beliefs anywhere in the code, or on our github repository. Do not add changes related to those topics.
* Do not curse extensively in pull requests/issues
* Do not bully someone for their github profile, bad code, profile picture, beliefs discovered on some personal page or anything that they done in the past
* If you have questions regarding code during a review, it's good to assume someone had a reason to do something, and it's better to ask "Why is it here, shouldn't we do XYZ?" rather than "What the f... is that garbage?"
* If your contribution is not getting attention, you can always ask people to take a look on the `#development` channel on our Discord, maybe someone will have some time and will give you a review. Just don't expect someone to do that immediately and don't spam the link, that may annoy people.
* Try to treat everyone with respect, this way we can develop things with professionalism, and in a faster, straight to the point way

If you see someone breaking the rules, contact one of the following maintainers that take a part in D1GT:
- [@AJenbo](https://github.com/AJenbo)
- [@StephenCWills](https://github.com/StephenCWills)
