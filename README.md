# Spark

The Client/Server for my Spark game that ran for several years in the 2000s.  

The game has been updated for 64 bit as it would not run when I first opened it up.  I updated to the latest Slick and lwjgl libraries that all support 64 bit.  The game/server run fine on Java 1.8 update 131 (current version when committed to repository).

I even fixed a couple of memory leaks that the new Eclipse Mylin detected that were present in the original version.

To get it running, simply download the project and open the workspace "Spark" with Eclipse.  There are 2 debug configurations already created for SparkServer and Spark.  Run SparkServer first, then Run the Spark debug configuration and it will connect you to a localhost server.
