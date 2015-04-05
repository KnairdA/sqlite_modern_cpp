# sqlite modern cpp wrapper

This library is aims to be a lightweight modern wrapper around the _sqlite_ C api.

## Example

A basic usage example is available in `example.cc` and may be compiled as follows:

```
mkdir build
cd build
cmake ..
make
```

Note that this requires _sqlite_ to be available in one of the standard system paths searched by _CMake_.

## Transactions

You can use transactions with `begin;`, `commit;` and `rollback;` commands.
*(don't forget to put all the semicolons at the end of each query)*.

```c++
db << "begin;"; // begin a transaction ...
db << "insert into user (age,name,weight) values (?,?,?);"
   << 20
   << u"bob"
   << 83.25f;
db << "insert into user (age,name,weight) values (?,?,?);" // utf16 string
   << 21
   << u"jack"
   << 68.5;
db << "commit;"; // commit all the changes.

db << "begin;"; // begin another transaction ....
db << "insert into user (age,name,weight) values (?,?,?);" // utf16 string
   << 19
   << u"chirs"
   << 82.7;
db << "rollback;"; // cancel this transaction ...
```

## License

MIT license - [http://www.opensource.org/licenses/mit-license.php](http://www.opensource.org/licenses/mit-license.php)
