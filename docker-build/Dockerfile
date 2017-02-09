FROM centos:7

MAINTAINER Don Bowman <don@sandvine.com>

RUN \
    yum -y install epel-release && \
    yum -y install git make gcc gcc-c++ autogen automake autoconf libtool file && \
    yum -y install rpm-build rpmdevtools && \
    yum -y install openpgm-devel python34-devel python34-Cython

CMD ["/bin/bash"]
