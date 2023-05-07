# os-projekt

## Opis

Projekt z Oprogramowania Systemowego w edycji 2022/23 - odzyskiwanie usuniętych plików w systemie plików FAT32. Docelowo pliki będzie można odzyskiwać poprzez *data carving* i korzystanie z informacji pozostałych w systemie plików.

Program w opcji odzyskiwania plików z użyciem informacji pozostałej w systemie plików zakłada, że pliki nie były sfragmentowane i odzyskuje dane z kolejnych klastrów pamięci, aż do klastra, który byłby ostatnim, jeśli plik o odczytanym rozmiarze zostałby zapisany ciągiem.

Opcja odzyskiwania plików przez *data carving* jest jedynie pokazowa (na cele projektu akademickiego) i odzyskuje tylko pliki `.png`, na podstawie znalezionej na dysku sygnatury początkowej tego typu plików. Należy w niej również podać limit rozmiaru, jakiej wielkości plików ma szukać, żeby program nie wykonywał się za długo, jeśli szukamy małych plików.

## Znane błędy

Obecnie program jest w wersji beta (ale raczej nie będzie już kontynuowany) i ma kilka błędów:
- nazwy odzyskiwanych plików nie są generowane (w opcji odzyskiwania z informacją z systemu plików), tylko przyjmowana jest część prawdziwej nazwy odzyskiwanego pliku - przez to dość często mogą zdarzać się kolizje w nazwach i odzyskany zostanie tylko 1 plik o podobnej nazwie
- lepiej, żeby nazwy odzyskiwanych plików nie przekraczały 8 znaków - nie zawsze działa dla długich nazw, ponieważ są one w inny sposób zapisywane w systemie FAT32 niż krótkie i program nie obsługuje takich sytuacji w pełni