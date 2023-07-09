# SENG
State engine library.

# Next change
- first edit the library.json to increment the patch level.

- next create branch.  'git branch -m (new branch)'
- commit and push 'git commit .'
    'git push origin (new branch)'

- remove old work area and install new branch.
   'cd ..'
   'rm -rf (old branch)'
   'git clone -b (new branch) https://github.com/jamlamb1959/SENG.git (new branch)'

- build the new branch.

  'cd (new branch)'
  'make'

- load into device
  'make mu'

- publish the changes.(Means the next change is a new branch.)
  'make publish'


