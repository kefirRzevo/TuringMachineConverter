# Emulating Turing Maching using Tag System

The original idea of ​​this program is to perform calculations using rule 110. But for this to be possible, it is necessary to carefully demonstrate the universality of this rule. To begin with, we will show that it is possible to perform calculations on a tag system, that is, its equivalence to a Turing machine.

## Turing machine
Turing machine is well-known mathematic model that is used for formalizing the
concept of algorithm.
There are some representations of Turing machine, such as state graph automata,
custom assembler or etc. Lets call `canonical` one that can be represented in suitable table:

| State         | On symbol       | Write symbol  | New state | Move
|:------------- |:---------------:| -------------:|-------------:|-------------:|
| a             | 0               | 1             | a            | L            |
| Row 2         | 1               | 0             | b            | R            |
| Row 3         | 0               | 0             | hlt          | R            |
| Row 3         | 1               | 1             | b            | L            |

This form of representation is convenient for parsing and is easy to read by humans.

## Tag system

The tag system is a certain alphabet and a queue over which operations are performed to remove two characters from the beginning and insert, according to special rules, the corresponding characters at the end.

For example we had line ``ACDA`` and have a rule for symbol `A` -> `CCA`. That means that the next state will be ``DACCA``.
So, first we need to show that using such a system it will be possible to perform one step of the Turing machine. This is described in more detail in the article *Matthew Cook "Universality in Elementary Cellular Automata"*

