# Suricata plugin for testimony integration


## Installation
Build using 
```bash
CPPFLAGS="-I/opt/suricata/src" make
```

where ```/opt/suricata/src``` is a path to suricata source code.

## Usage 

Add to suricata.yaml configuration file:

```
plugins:
  - /path/to/testimony/library/source-testimony.so
testimony:
  /tmp/testimony.sock:
    fanout-size: 4
```

Where  ```/tmp/testimony.sock``` is a path to testimony socket and ```fanout-size``` is a fanout size configured in the testimony configuration file.


 Run suricata using options ```--capture-plugin``` and ```--capture-plugin-args```:

```bash
suricata -c suricata.yaml --capture-plugin testimony-plugin --capture-plugin-args /tmp/testimony.sock
```

where ```/tmp/testimony.sock``` is a path to testimony socket
