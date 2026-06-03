# RDBMS - Structured Query Language (SQL) Examples

## Bootstrap the Runtime Environment

### Load SQLite

In order to work with SQLite in Pyodide, we will need to load the relevant package. Run the following code.

:::{pyodide-cell}
:id: sqlite-load

import pyodide_js
await pyodide_js.loadPackage("sqlite3")
:::

_You should see the following output:_
```{code} sh
[object Object]
```

### Connect to a DB and Create a Cursor

We will need to create our university student course enrollment database. The call to `connect()` will create a connection to the specified database and implicitly create it if the database doesn't exist.

To execute SQL statements and run SQL queries, we need a database cursor. This cursor will be used in the following exercises.

:::{pyodide-cell}
:id: sqlite-connect

import sqlite3
con = sqlite3.connect("university.db")
cur = con.cursor()

:::

## Interactive Examples

### CREATE the `students` Table

:::{tip} Example
Create a table a 'students' table, if it doesn't already exist, to store the Physics Department student records with the following attributes:

- id INTEGER PRIMARY KEY AUTOINCREMENT,
- first_name VARCHAR(255) NOT NULL,
- last_name VARCHAR(255) NOT NULL,
- major VARCHAR(255) NOT NULL,
- age INTEGER
:::

:::{pyodide-cell}
:id: create-table-example

# Avoid the error for an existing table with 'IF NOT EXISTS".
cur.execute("""
    CREATE TABLE IF NOT EXISTS students (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        first_name VARCHAR(255) NOT NULL,
        last_name VARCHAR(255) NOT NULL,
        major VARCHAR(255) NOT NULL,
        age INTEGER
    )
""")

# The following will verify that the table has been created by querying
# the built-n `sqlite_master` table and returning a tuple of the table names.
res = cur.execute("SELECT name FROM sqlite_master")
print(res.fetchone()[0])

:::

_You should see the following output:_
```{code} sh
students
```

### INSERT Records into the `students` Table
:::{tip} Example
Let's add some students.
:::

:::{pyodide-cell}
:id: insert-example

cur.execute("""
    INSERT INTO students (first_name, last_name, major, age)
    VALUES
        ('Mike', 'Clymer', 'Computer Science', 20),
        ('Anna', 'Schmidt', 'Physics', 18),
        ('Bella', 'Cruz', 'Math', 19),
        ('Lee', 'Jacobs', 'Physics', 21)
""")

con.commit() # Don't forget to commit.
:::

:::{tip} Example
Now we can fetch all of the student records and order them by age.
:::

:::{pyodide-cell}
:id: select-all-example

for row in cur.execute("SELECT * FROM students ORDER BY age"):
    print(row)

:::

_You should see the following output:_
```{code} sh
(2, 'Anna', 'Schmidt', 'Physics', 18)
(3, 'Bella', 'Cruz', 'Math', 19)
(1, 'Mike', 'Clymer', 'Computer Science', 20)
(4, 'Lee', 'Jacobs', 'Physics', 21)
```
:::{tip} Example
Get a count of the total number of students.
:::

:::{pyodide-cell}
:id: count-example

print(cur.execute("SELECT COUNT(*) FROM students").fetchone()[0])
:::

_You should see the following output:_
```{code} sh
4
```

:::{tip} Example
Get a count of the number of students that are Physics majors.
:::

:::{pyodide-cell}
:id: count-example

res = cur.execute("SELECT COUNT(*) FROM students WHERE major='Physics'")
print(res.fetchone()[0])

:::

_You should see the following output:_
```{code} sh
2
```
