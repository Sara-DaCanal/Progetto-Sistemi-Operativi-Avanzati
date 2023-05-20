# Progetto-Sistemi-Operativi-Avanzati
## Progetto per il corso di Sistemi Operativi Avanzati dell'Università di Roma Tor Vergata
__Autore__
* :woman_technologist: Sara Da Canal (matricola 0316044)

## Introduzione
Lo scopo del progetto è creare un file system, basato su un dispositivo a blocchi e che contenga un unico file.
L'architettura considerata prevede due inode, uno per la root del file system, ovvero la directory dove si trova il file, e un inode child per il file vero e proprio.

Le operazioni implememtate sono le seguenti:
* Operazioni a livello del blocco:
  1. put_data: inserisce i dati passati in input in un blocco libero
  2. get_data: fornisce in output i dati di uno specifico blocco
  3. invalidate_data: cancella i dati di uno specifico blocco
* Operazioni a livello del file:
  1. open: apre il file come uno stream
  2. release: rilascia il file
  3. read: legge i blocchi in ordine temporale
* Operazioni a livello dell'inode:
  1. lookup: per leggere le informazioni relative al file e far funzionare il comando "ls" sulla directory
* Operazioni a livello della directory:
  1. Iterate: per leggera la directory

## Mount del file system
La registrazione del file system e le funzioni di montaggio possono essere trovate in [user_message_fs_create.c](./user_message_fs_create.c "User_message_fs_create"). Il file system deve avere un'unica istanza montata, questo viene gestito tramite una variabile globale del modulo che viene incrementata in modo atomico durante il montaggio e decrementata nello stesso modo durante lo smontaggio. Per il montaggio è stata usata la funzione mount_bdev(), che serve a montare un generico block device, ma la funzione per la fill del superblocco è stata sovrascritta. La nuova fill include il controllo sul massimo numero di blocchi supportati, e il caricamento dei metadati per il device in ram. In particolare, vengono caricati alcuni metadati relativi all'intero device e una lista RCU di metadati dei blocchi validi, questa ci permette di effettuare le operazioni di lettura senza bisogno di un lock. Per trovare facilmente i blocchi liberi si usa una bitmask di tutti i blocchi con i validi settati a 1.

## Operazioni sui blocchi
Per implementare queste operazioni è stato necessario aggiungere tre nuove system call su entry libere della system call table. La ricerca di entry libere è stata fatta tramite il modulo "the usctm", che esporta l'indirizzo della system call table e un array di entry disponibili come parametri. Questi due parametri sono stati passati al mio modulo in fase di registrazione, e usati per registrare tre system call ([syscall_table_searcher.c](./syscall_table_searcher.c "Syscall_table_searcher") ).
Le system call registrate controllano che il device sia montato, in caso di risposta affermativa chiamano l'implementazione vera e propria delle operazioni, situata nel file [umsg_block.c](./umsg_block.c), a cui vengono passati gli stessi parametri delle system call più il superblocco del file system, in caso negativo ritornano con errore ENODEV. 

La funzione di put prevede di cercare un blocco libero tramite la bitmask, prendendo un lock prima di iniziare per evitare che put diverse scelgano lo stesso id. Il bit viene settato prima di rilasciare il lock. A questo punto il blocco corrispondente viene letto dalla memoria e caricato in un buffer_head, i dati sul buffer_head vengono modificati e il buffer_head viene marcato per la riscrittura sul device. Viene allocato lo spazio per una nuova entry nell'RCU list e, dopo aver preso il lock in scrittura sulla lista, viene aggiunto il blocco.

La funzione di get prevede di scorrere l'RCU list dei metadati fino a che non si trova il giusto blocco, e poi quel blocco viene letto dalla memoria. Non sono necessari lock in lettura perché sfruttiamo le API delle RCU list.

La funzione di invalidate cerca il blocco da invalidare nella lista dei metadati, quando il blocco viene trovato prende il lock in scrittura e lo cancella dalla lista, poi rilascia il lock. A quel punto il blocco viene letto dal device e il bit di validità viene resettato, il buffer_head usato per la lettura viene marcato per la riscrittura sincrona o asincrona sul device. A questo punto viene resettato in modo atomico il bit corrispondente al blocco invalidato sulla maschera mantenuta tra i metadati

## Operazioni sui file
Queste operazioni si possono trovare nel file [umsg_file.c](./umsg_file.c "umsg_file"). Il file in lettura viene visto come una sorta di pipe, il file aperto come uno stream non mantiene informazioni sulla posizione all'interno del file, e una volta aperto è soltanto leggibile dall'inizio alla fine senza possibilità di tornare indietro finché non viene chiuso e riaperto. Una singola lettura legge dati dal file finché i dati sono presenti e il buffer è abbastanza grande per contenerli. In fase di apertura viene impostato un timestamp a zero, quando avviene una lettura questo timestamp viene impostato a quello dell'ultimo blocco letto, in modo che alla lettura successiva si riparta dal blocco successivo. Se la lunghezza di un blocco è maggiore rispetto allo spazio disponibile sul buffer quel blocco non viene letto. La lettura viene effettuata tramite buffer_head seguendo l'ordine con cui i blocchi sono nella lista rcu e come per la get non sono necessari lock.

## How to 
 * Compilazione con "make"
 * Inserimento del modulo con "sudo make insmod", perché l'inserimento vada a buon fine è necessario che il blocco "The usctm" sia caricato
 * Formattazione del device con "make format"
 * Montaggio del device con "sudo make mount-fs"
 * Smontaggio del device con "sudo make umount-fs"
 * Rimozione del modulo con "sudo make rmmod"
 
 Il file [my_client.c](./my_client.c "My_client") può essere usato per testare.
