# XbarGen
<div class="col-md-8 text-left" style="max-width: 825px">
          <h1 style="margin-left: -30px;" id="xbargen">XbarGen - Overview</h1>
          <p>XbarGen is an open-source software system for synthesizing memristor-based digital circuits.</p>
          <p>Specifically, XbarGen executes a translation of a boolean function into a digital circuit. Particularly, it targets a specific memristor-based crossbar architecture.</p>
          <p>The underlying idea behind XbarGen is performing a design space exploration of memristor-based crossbar architectures.</p>
          <p>XbarGen has been developed from scratch, due to the lack of an automated process of translation from a given boolean function to a memristor-based crossbar architecture.</p>
          <p>XbarGen is part of a far-reaching project, yet at its very early stage, which aims to realize a Computer-Aided-Design (CAD) software system that targets memristor architectures.</p>
          <p>XbarGen is licensed with the GNU General Public License (GNU GPL). Please visit <a href="http://www.gnu.org/licenses/">http://www.gnu.org/licenses/</a> to see a copy of the license.</p>
          <p>Although XbarGen is a project in which <a href="acknowledgements#team">different people</a> are involved,
            we kindly ask you to direct bug reports, questions and other comments to <a href="http://www.lirmm.fr/~traiola/">Marcello Traiola</a>
            or to the Bitbucket issue tracker.</p>
            <h2 id="buil" style="margin-left: -30px;">Building</h2>
            <p>Before getting started, there are some requirements that are expected to be satisfied:</p>

            <div class="panel panel-default">
              <div class="panel-heading">Requirements</div>
              <div class="panel-body" style="margin-left: -15px;" >
                <ul>
                  <li>We strongly suggest a <b>unix-like</b> system for compiling and using XbarGen;</li>
                  <li>We used <a href="https://cmake.org">CMake</a>, for building the Makefile. The oldest version with which we tested is the 2.8.12.2 but it should reasonably work also with other versions;</li>
                  <li>Finally <b>g++</b> is mandatory, for compiling XbarGen. The oldest version with which we tested is the 4.8.4 (Ubuntu - 2013) but it should reasonably work also with other versions.</li>
                  <li>Make sure that the paths of 'cmake' and 'g++' binary folders are included within the PATH environment variable</li>
                </ul>
              </div>
            </div>
            <p>To compile XbarGen, you need to download the source code, open the shell, move to the downloaded folder and run the following commands:</p>
            <code class="sh hljs bash">$ <span class="hljs-built_in">mkidr</span> build <br/>
              $ <span class="hljs-built_in">cd</span> build <br/>
              $ <span class="hljs-built_in">cmake</span> .. </br/>
              $ <span class="hljs-built_in">make</span></code>
              <p>In case you receive compilation errors, they should be self-explanatory.</p>
              <p>You should receive the following messages after executing the 'make' command</p>
              <p><img src="figures/compilazione.png" /> </p>
              <p>If the compilation succeeds, the same directory contains the executables, namely <code class="hljs-built_in">XbarGen</code>.</p>
              <h2 id="doc" style="margin-left: -30px;">Documentation</h2>
              <p>XbarGen is released with documentation. In particular, the source code comes along with <a href="http://doxygen.org">Doxygen</a> documentation.</p>
              <p>Doxygen internal documentation can be found <a target="_blank" href="internal-doc">here</a></p>
              <p>It is also possible to generate XbarGen's internal documentation in order to consult it 'offline'</p>
              <div class="panel panel-default">
                <div class="panel-heading">Requirements</div>
                <div class="panel-body" style="margin-left: -15px;">
                  <ul>
                    <li>Install <a href="http://doxygen.org">Doxygen</a> (oldest version tested: 1.8.9.1)</li>
                    <li>Install <a href="http://www.graphviz.org">Graphviz</a> (oldest version tested: 2.36)</li>
                    <li>Make sure that the paths of 'doxygen' and 'dot' binary folders are included within the PATH environment variable</li>
                  </ul>
                </div>
              </div>
              <p>CMake will eventually prepare the an entry for generating a Doxygen documentation project and you just need to type: </p>
              <code class="sh hljs bash">$ <span class="hljs-built_in">make</span> doc</code>
              <p>in the building folder to generate it. The documentation folder will be created in the building directory's parent folder.</p>
            </div>
