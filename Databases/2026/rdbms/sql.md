# RDBMS - SQL Exercises

## Bootstrap the Exercise Runtime Environment

### Load SQLite

In order to work with SQLite in Pyodide, we will need to load the relevant package. Run the following code.

:::{pyodide-cell}
:id: sqlite-load

import pyodide_js
await pyodide_js.loadPackage("sqlite3")
:::

You should see the following output:
```{code} sh
[object Object]
```

### Connect to the 'higher-ed' SQLite DB

We will need to create our higher education management system database.

:::{pyodide-cell}
:id: sqlite-connect
import sqlite3
con = sqlite3.connect("higher-ed.db")
:::

## Exercises

```{exercise}
:label: exercise-1

Let's get started
```

:::{pyodide-cell}
:id: solution-input
def factorial(number: int):
    pass

factorial(4)
:::

````{solution} exercise-1
:label: my-solution-1
:class: dropdown

Here's one solution.

```{code} python
:linenos:
:emphasize-lines: 2,3,4,5

def factorial(n):
    k = 1
    for i in range(n):
        k = k * (i + 1)
    return k

factorial(4)
```
````

### CREATE a New Table

:::{pyodide-cell}
:id: sqlite-create-table
cur = con.cursor()
cur.execute("CREATE TABLE movie(title, year, score)")

res = cur.execute("SELECT name FROM sqlite_master")
res.fetchone()

res = cur.execute("SELECT name FROM sqlite_master WHERE name='spam'")
res.fetchone() is None
:::

### INSERT Records into the Table
:::{pyodide-cell}
:id: sqlite-insert
cur.execute("""
    INSERT INTO movie VALUES
        ('Monty Python and the Holy Grail', 1975, 8.2),
        ('And Now for Something Completely Different', 1971, 7.5)
""")

con.commit() # Don't forget to commit.
:::


### SELECT Records from the Table
:::{pyodide-cell}
:id: sqlite-select
res = cur.execute("SELECT score FROM movie")
res.fetchall()
:::
