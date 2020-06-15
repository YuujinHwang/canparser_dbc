can-dbcparser
=============

Short Project to parse the DBC files used by Vector for CAN

# Compilation

```
mkdir build
cd build
cmake ..
make
```

# Usage

```
 ./candbcparser canmatrix.dbc
 ```

# Decode Message

After create dbc object with proper .dbc file,

```
dbc.decode(MSG_ID, PAYLOAD)[SIGNAME]

#example
dbc.decode(688, 77426917114)["AngleSteer"]
```
this will return decoded(casted, scaled and offset shifted) value in (double)
