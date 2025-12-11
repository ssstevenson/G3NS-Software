# G3-NewSoftware
<H1>CMake Build Setup</H1>
<H2>Basic Build Process for All</H2>
cd .../Gen3Booster/software</br>
mkdir build/</br>
cd build/</br>
cmake ..</br>
make -j</br></br></br>
<H2>Basic Build Process Individual Process</H2>
cd .../Gen3Booster/software</br>
mkdir build/</br>
cd build/</br>
cmake ..</br>
make &lt;PROCESS&gt; -j</br></br></br>
<H2>Deploy to Target</H2>
cd .../Gen3Booster/software</br>
mkdir build/</br>
cd build/</br>
cmake -DREMOTE_TARGET=&lt;IP Address&gt; ..</br>
make deploy-&lt;PROCESS&gt; -j</br>
