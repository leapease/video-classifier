# **Minerva Technical Documentation**

## Overview

This project implements a general purpose classification system.  The focus of
the project is on achieving robust performance (classification accuracy) with
minimal human interaction. Unlike most classification systems, which rely on
domain specific heuristics to perform feature selection, Minerva uses an
unsupervised learning technique called 'sparse autoencoding' that learns
important features automatically over time without any human interaction.
The sparse autoencoder is implemented with a convolutional neural network that takes raw
data as input and produces a set of features that attempt to capture the
essential information in that data.  It is trained by streaming through massive
unlabeled input data sets. Once the sparse autoencoder has learned useful
features, it is connected to a more traditional classification system that
is trained using supervised learning.  This system is also implemented with
a neural network, and it attempts to discover complex relationships between
generated features and output classes.  

Although Minerva is designed the handle arbitrary input data, this project
includes additional supporting modules for performing classification on video
data. As our first case study, we are using Minerva to perform automatic gesture
recognition.  

The labeled dataset used for training was obtained from http://www.kaggle.com/c/multi-modal-gesture-recognition

## 1. Unsupervised Learning

Unsupervised learning (or 'deep learning') is used to describe the process of
automatically discovering patterns in large data sets without human interaction.
Without unsupervised learning, it is necessary to manually label thousands
or millions of video frames to perform classification (identify what an image
contains) that is robust to variations in color, alignment, and noise.  This
is not feasible for most project since developers do not have the resources
to gather such a large number of videos.  

As a response, many projects rely
on building domain-specific feature selection systems that preprocess the
data and produce a set of features or metrics that attempt to capture the
essential information in the input data set using a significantly smaller
representation.  Feature selection is effective at reducing the dependence on
labeled data because it simplifies the problem being presented to the supervised
learning system (i.e. determine the class of a dataset using only the most
relevant information rather than all information), and Sift is an example
of a widely used feature selection system.  Despite their usefulness, 
manually-crafted feature selection systems have a fatal flaw; they are designed
by developers and tailored to specific problems.  As a consequence, they
are often brittle (because they are designed using developer intuition),
and require a complete redesign when moving from one type of input data to
another (e.g. from video to text).

A sparse autoencoder is an unsupervised learning technique that attempts to
address the shortcomings of feature selection systems by providing a framework
that generates them automatically.  At a high level, it works using an
artificial neural network that is trained with unlabaled data and configured
with a very specific topology that guides that inner layers of the network
to respond to patterns in the input data that capture the majority of
information in that data.  

The details of sparse autoencoders are described here
http://stanford.edu/class/cs294a/sparseAutoencoder.pdf .  

## 2. Supervised Learning

Minerva uses a well known neural network for supervised learning.  The input
data is preprocessed using the feature selection network.  

## 3. Classification

Once a network has been trained, classification is performed simply by
propagating the input data through the feature selection and classification
neural networks to obtain a predicted class.

## 4. High Performance Architecture

Minerva is designed with high performance in mind to be able to scale to large
data sets. It uses three principles to achieve high performance.  

* Use high performance accelerator architectures (e.g. GPUs) in single nodes.
* Use a scalable design that can leverge distributed systems.
* Design convolutional neural networks in the first few layers to reduce the computational complexity of large networks.

A long term vision for this project is to scale to millions of input features
on single node systems, and billions of input features on thousand node
clusters.

# System
The system is composed of the following main component libraries:

* The artificial neural network
* The optimization library
* The linear algebra library
* The video library

These libraries are used internally by the following interfaces:

* The model builder
* The supervised learning module
* The unsupervised learning module
* The classification module

In Minerva, the classification system is called a Model.  Models contain a serialized representation
of the entire system, including all neural networks and associated metadata.  The model builder is used to create a blank model with new parameters (network topology, labels, etc).

Once a model has been created, it can be updated with supervised or unsupervised learning, or it can be used directly to perform classification.  Minerva currently supports video and image input data sets, but none of the modules are tied to this representation.  Supporting a new input format (e.g. text or numeric data) would simply require a new driver application that converts the input into a standardized vector of floating point values, and presents it to the existing models.

The details of the libraries and modules are described next.

### The Neural Network Library

 Each neural network contains multiple layers. Each layer is represented as a collection of sparse matrices.  The neural network library starts by initializing the network randomly. The inputs to the neural network are pixels of a down-sampled image in the form of a matrix. With pixel values as input to the neural network, we use the back-propagation algorithm to train the neural network. The neural network is trained until the cost function for the neural network is minimized. The optimization library is used to compute this cost function. 

### The optimization library

 The optimization library aims to reduce the difference between the actual output (from the labeled data) and the output predicted by the neural network. This library contains a couple of implementations. 
a.) Gradient descent with linear simulated annealing
b.) The Multilevel optimizer uses a greedy heurisitc with simulated annealing & local search (with tabu search)

### The linear algebra library

 The linear algebra library leverages the optimized Matrix operations from pre-existing implementations. The smallest unit for calculations in the neural network & optimizer is the Matrix. The linear algebra library translates these into calls to the Matrix library.

### The video library

 OpenCV is used for all the image processing required in this project. The input video is converted into a series of images. These images are then converted to a lower resolution & then finally to a matrix of pixel values. 

### The model builder

 At each of the following steps, the neural network generated is written out to the disk. The network is a represented as a model with various attributes & their corresponding values. This model is then serialized & written to a file.
 Writing these models decouples each step & thus allows the capability of resuming with the help of a model file. Eg: The unsupervised learning step takes many hours of running video to automatically generate a feature selector neural network. This network can be saved to a file & then reused with different sets of training data to create a classification neural network. This saves the time required to rerun the unsupervised learning step.

### The unsupervised learning module

 This is the first step. We start by creating a new model. This new model is simply a randomly initialized neural network. We then feed video database to this network. After processing sufficiently large amounts of input images the neural network automatically configures itself to be a feature selector. Here are some details of how the neural network is configured:
The idea behind a sparse autoencoder is that features in some data can be attributed to only some relevant parts of the input space, rest of it is noise. Using a steady reduction factor, the size of the neural network layers is decreased significantly. At this stage, we start adding new layers one at the time and train all the neurons in the new layer by running back-propagation algorithm.

### The supervised learning module

 This module uses the neural network from the earlier unsupervised learning stage as the starting point (instead of a randomly initialized neural network). At the start, this neural network is capable to selecting features. We feed in labeled data to this network to generate a classifier neural network. This is the training step which uses labeled data with gesture names for images. At the end of this step i.e. once the neural network output is within threshold of the expected output, it is written out as a classifier neural network ready to be used for testing with images.

### The classification module


