FROM ubuntu:18.04

RUN apt-get update && \
    apt-get install -y -qq software-properties-common apt-utils && \
    rm -rf /var/lib/apt/lists/* && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get install -y -qq build-essential \
                       tcsh \
                       libtool-bin \
                       libtool \
                       automake \
                       gfortran-9 \
                       libglu1-mesa-dev \
                       libfreetype6-dev \
                       uuid-dev \
                       libxmu-dev \
                       libxmu-headers \
                       libxi-dev \
                       libx11-dev \
                       libxml2-utils \
                       libxt-dev \
                       libjpeg62-dev \
                       libxaw7-dev \
                       liblapack-dev \
                       git \
                       gcc-9 \
                       g++-9 \
                       libgfortran-9-dev \
                       curl \
                       wget \
                       python-pip \
                       python3-pip \
                       libblas-dev \
                       zlib1g-dev \
                       mpich \
                       libboost-all-dev \
                       libfltk1.3-dev \
                       libeigen3-dev \
                       qt5-default \
                       libqt5x11extras5-dev \
                       xxd
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 50 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 50
RUN pip3 install --upgrade pip

ARG working_dir=/home/freesurfer
WORKDIR $working_dir

# Get gems python code and requirements
COPY . $working_dir/
#COPY python $working_dir/python

#RUN cd $working_dir/gems && git clone https://github.com/pybind/pybind11.git
#RUN pip3 install -r $working_dir/python/requirements.txt

#Install cmake
RUN curl -O https://cmake.org/files/v3.16/cmake-3.16.5-Linux-x86_64.sh && sh ./cmake-3.16.5-Linux-x86_64.sh --skip-license && cp -r bin /usr/ && cp -r doc /usr/share/ && cp -r man /usr/share/ && cp -r share /usr/

RUN mkdir -p cmake-build-debug-bionic && cd ./cmake-build-debug-bionic && \
    wget "https://www.dropbox.com/s/5xffk87vm0wb938/linux_packages.tar?dl=1" >/dev/null 2>&1 && \
    tar -xf "./linux_packages.tar?dl=1" && \
    rm -rf ./prebuilt_packages/itk && \
    wget "https://www.dropbox.com/s/uqysekgefm3vb8x/itk5.0.1_linux_bionic.tar?dl=1" >/dev/null 2>&1 && \
    tar -xf "./itk5.0.1_linux_bionic.tar?dl=1" && \
    mv ./itk5 ./prebuilt_packages/itk

# Build gems
ENV HOSTING_OS bionic

RUN cd ./cmake-build-debug-bionic && \
    cmake -D FS_BUILD_GUIS=OFF \
          -D BUILD_TESTING=OFF \
          -D CMAKE_BUILD_TYPE=Release \
          -D PYTHON_EXECUTABLE=/usr/bin/python3.6 \
          -D PYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.6m.so \
    .. && \
    make -j8
