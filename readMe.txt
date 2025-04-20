Multi Train Railway System 
****Authors:
Ryeleigh Avila
Roberts Kovalonoks
Yosep Lazar
Judah Lomo
Joey Mecoy
****Course: CS 4323 Operating systems
****Project: Railway Simulation with IPC and Deadlock handling

****Overview:
Our files will demonstarte a railway system where multiple trains (Child processes) traverse shared intersectioins. It will utilize inter process communication (IPC), semaphores, mutexes, shared memory, and deadlock detection/resolution. Here each train requests access to intersections and parent process grants or delays access based on availability. The events are logged with timestamps in a file called simulation.log.

****Input files:
-Intersection.txt
Format: IntersectionName: Capacity
Example: 
intersectionA:1
intersectionB:2

-trains.txt
Format: TrainName:Intersection1,Intersection2
Example:
Train1:IntersectionA,IntersectionB,IntersectionC


****Compiling (Compile program, run): 
g++ -o railway_sim parent_process.cpp IPC_Setup.cpp logger.cpp deadlock_resolution.cpp Deadlock_Detection.cpp -pthread
g++ -o train_process Train_process.cpp -pthread

*****Running(start simulation):
./railway_sim
Make sure inetrsection.txt and trains.txtx are in same directory


****Output:
Still testing ans working on it
