# Creating a new major release

Update the version in cap32.h:

```
$ vi src/cap32.h
(...)
#define VERSION_STRING "vX.Y.0"
(...)
```

Tag the release:

```
$ git tag vX.Y.0
```

Push the result:

```
$ git push
```

Delete the latest release from github, it will be recreated by the next push.
It's probably a good idea to immediately trigger a build by pushing a dummy change.

# Creating a new minor release

Create a branch from the existing tag and switch to it:

```
$ git checkout -b vX.Y vX.Y.0
```

Do the required modifications and then tag it. Push the result:
```
$ git tag vX.Y.0
$ git push
```

Verify that appveyor releases it properly.

Delete the latest release from github, it will be recreated by the next push.
It's probably a good idea to immediately trigger a build by pushing a dummy change.
