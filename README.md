## Kernel running in QEMU

* ``git clone`` this repository
* navigate to ``/``
* open the ``Makefile`` located in ``/``
* edit the ``NASM``, ``AS`` and ``CC`` variables
* run ``make``
* run ``setup_ethernet.sh``
* run ``run.sh``

## ``NASM``, ``AS`` and ``CC``

* Set the ``NASM`` variable to the NASM assembler executable.
* Set the ``AS`` and ``CC`` variables to the cross-compiler/cross-assembler executables.

Works with the following cross-compiler:
* gcc version 10.2.0 compiling a gcc version 10.2.0 cross-compiler
* ``--target=i686-elf``
* ``--disable-nls``
* ``--enable-languages=c,c++``
* ``--without-headers``

## ``setup_ethernet.sh``

* Creates a ``br0`` bridge
* Creates a ``tap0`` tap
* Connects the ``eth0`` interface and ``tap0`` to ``br0``
* Queries for an ip address for ``br0``

The obvious assumption here is that the ethernet interface is called ``eth0``.

## ``run.sh``

* Runs ``elf_reader.py``
* Concatenates the bootloader and kernel binaries.
* Runs ``make_large.py``
* Runs ``QEMU``

``elf_reader.py`` extracts the kernel binary from the ``.elf`` file.
``make_large.py`` inflates the concatenated binary to ``1MB`` (QEMU wouldn't recognise it as an AHCI device otherwise).