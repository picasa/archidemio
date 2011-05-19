/*
 * Copyright (C) 2011 - INRA
 * Gauthier Quesnel <quesnel@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided "as is", without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising
 * from, out of or in connection with the software or the use or other dealings
 * in the Software.
 */

#include "GraphTranslator.hpp"
#include <vle/utils/Exception.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <vector>
#include <map>
#include <set>

namespace detail {

class Node
{
public:
    Node()
        : name(), index(-1), classname()
    {
    }

    Node(const std::string& name, int index)
        : name(name), index(index), classname()
    {
    }

    Node(const std::string& name, int index, const std::string& classname)
        : name(name), index(index), classname(classname)
    {
    }

    bool operator<(const Node& node) const
    {
        return name < node.name;
    }

    bool operator==(const Node& node) const
    {
        return name == node.name;
    }

    const std::string& getName() const
    {
        return name;
    }

    int getIndex() const
    {
        return index;
    }

    const std::string& getClassName() const
    {
        return classname;
    }

    std::string name;
    int index;
    std::string classname;
};

class NodeList
{
public:
    typedef std::vector < Node > value_type;
    typedef std::vector < Node >::iterator iterator;
    typedef std::vector < Node >::const_iterator const_iterator;
    typedef std::vector < Node >::size_type size_type;

    NodeList()
        : mLst()
    {
    }

    void resize(const std::string& prefix, int number)
    {
        mLst.resize(number);

        iterator it = begin();
        int i = 0;

        while (it != end()) {
            it->name = (boost::format("%1%-%2%") % prefix % i).str();
            it->index = i;
            it->classname.clear();
            it++;
            i++;
        }
    }

    void insert(const std::string& name, int index)
    {
        mLst.push_back(Node(name, index));
    }

    Node& operator[](int index)
    {
        if (index < 0 or index >= (int)size()) {
            throw vle::utils::ArgError();
        }

        return mLst[index];
    }

    const Node& operator[](int index) const
    {
        if (index < 0 or index >= (int)size()) {
            throw vle::utils::ArgError();
        }

        return mLst[index];
    }

    void replace(const std::string& name, int index,
                 const std::string& classname)
    {
        if (index < 0 or index >= (int)size()) {
            throw vle::utils::ArgError();
        }

        mLst[index] = Node(name, index, classname);
    }

    iterator begin()
    {
        return mLst.begin();
    }

    iterator end()
    {
        return mLst.end();
    }

    const_iterator begin() const
    {
        return mLst.begin();
    }

    const_iterator end() const
    {
        return mLst.end();
    }

    size_type size() const
    {
        return mLst.size();
    }

private:
    value_type mLst;
};

} // namespace detail

/*
 *
 * Connection implementation
 *
 */

Connection::Connection() :
    modelSource(),
    portSource(),
    modelDestination(),
    portDestination()
{
}

Connection::Connection(const std::string& modelSource,
                       const std::string& portSource,
                       const std::string& modelDestination,
                       const std::string& portDestination) :
    modelSource(modelSource),
    portSource(portSource),
    modelDestination(modelDestination),
    portDestination(portDestination)
{
}

Connection::Connection(const Connection& other) :
    modelSource(other.modelSource),
    portSource(other.portSource),
    modelDestination(other.modelDestination),
    portDestination(other.portDestination)
{
}

Connection::~Connection()
{
}

Connection& Connection::operator=(const Connection& other)
{
    Connection tmp(other);

    std::swap(modelSource, tmp.modelSource);
    std::swap(portSource, tmp.portSource);
    std::swap(modelDestination, tmp.modelDestination);
    std::swap(portDestination, tmp.portDestination);

    return *this;
}

/*
 *
 * DynamicGraphTranslator implementation
 *
 */

class DynamicGraphTranslator::Pimpl
{
public:
    Pimpl(const vle::value::Map& buffer)
        : mConnectionType(CONNECTION_TYPE_IN_NAMED),
        mPrefix("node"),
        mDefaultClassName("class")
    {
        init(buffer);
    }

    void build(vle::devs::Executive* model)
    {
        reinit();

        assert(mCacheMatrix.rows() == mCacheMatrix.cols());
        assert(mCacheMatrix.rows() >= mCurrentMatrix.rows());

        if (mCacheMatrix.rows() > mCurrentMatrix.rows()) {
            for (int i = mCacheMatrix.rows() - mCurrentMatrix.rows();
                 i < mCacheMatrix.rows(); ++i) {
                createNode(model, i);
            }
        }

        for (int j = 0; j < mCacheMatrix.cols(); ++j) {
            for (int i = 0; i < mCacheMatrix.rows(); ++i) {
                if (i > mCurrentMatrix.rows() or mCacheMatrix(j, i) !=
                    mCurrentMatrix(j, i)) {
                    if (mCacheMatrix(j, i) != 0.0) {
                        createConnection(model, i, j);
                    } else {
                        removeConnection(model, i, j);
                    }
                }
            }
        }

        mCurrentMatrix = mCacheMatrix;
    }

    ConnectionTypeOptions mConnectionType;
    std::string mPrefix;
    std::string mDefaultClassName;
    detail::NodeList mAssociation;
    Eigen::MatrixXd mCurrentMatrix;
    Eigen::MatrixXd mCacheMatrix;
    ConnectionList mNewConnection;
    ConnectionList mDeletedConnection;
    ModelNameList mNewModel;
    ModelNameList mDeletedModel;

private:
    void reinit()
    {
        mNewConnection.clear();
        mDeletedModel.clear();
        mNewModel.clear();
        mDeletedModel.clear();
    }

    void createNode(vle::devs::Executive* model, int index)
    {
        const detail::Node& node = mAssociation[index];

        if (model->coupledmodel().findModel(node.getName()) == 0) {
            model->createModelFromClass(
                node.getClassName().empty() ? mDefaultClassName :
                node.getClassName(),
                node.getName());
            mNewModel.push_back(node.getName());
        }
    }

    void removeNode(vle::devs::Executive* model, int index)
    {
        const detail::Node& node = mAssociation[index];

        if (model->coupledmodel().findModel(node.getName()) != 0) {
            model->delModel(node.getName());
            mDeletedModel.push_back(node.getName());
        }
    }

    void createConnection(vle::devs::Executive* model, int to, int from)
    {
        switch (mConnectionType) {
        case CONNECTION_TYPE_IN_OUT:
            model->addOutputPort(mAssociation[from].getName(), "out");
            model->addInputPort(mAssociation[to].getName(), "in");
            model->addConnection(mAssociation[from].getName(), "out",
                                 mAssociation[to].getName(), "in");

            mNewConnection.push_back(
                Connection(mAssociation[from].getName(),
                            "out",
                            mAssociation[to].getName(),
                            "in"));
            break;
        case CONNECTION_TYPE_IN_NAMED:
            model->addOutputPort(mAssociation[from].getName(),
                                 mAssociation[to].getName());
            model->addInputPort(mAssociation[to].getName(), "in");
            model->addConnection(mAssociation[from].getName(),
                                 mAssociation[to].getName(),
                                 mAssociation[to].getName(), "in");

            mNewConnection.push_back(
                Connection(mAssociation[from].getName(),
                            mAssociation[to].getName(),
                            mAssociation[to].getName(),
                            "in"));
            break;
        case CONNECTION_TYPE_NAMED_OUT:
            model->addOutputPort(mAssociation[from].getName(), "out");
            model->addInputPort(mAssociation[to].getName(),
                                mAssociation[from].getName());
            model->addConnection(mAssociation[from].getName(), "out",
                                 mAssociation[to].getName(),
                                 mAssociation[from].getName());

            mNewConnection.push_back(
                Connection(mAssociation[from].getName(),
                            "out",
                            mAssociation[to].getName(),
                            mAssociation[from].getName()));
            break;
        case CONNECTION_TYPE_NAMED:
            model->addOutputPort(mAssociation[from].getName(),
                                 mAssociation[to].getName());
            model->addInputPort(mAssociation[to].getName(),
                                mAssociation[from].getName());
            model->addConnection(mAssociation[from].getName(),
                                 mAssociation[to].getName(),
                                 mAssociation[to].getName(),
                                 mAssociation[from].getName());

            mNewConnection.push_back(
                Connection(mAssociation[from].getName(),
                            mAssociation[to].getName(),
                            mAssociation[to].getName(),
                            mAssociation[from].getName()));
            break;
        }
    }

    void removeConnection(vle::devs::Executive* model, int to, int from)
    {
        switch (mConnectionType) {
        case CONNECTION_TYPE_IN_OUT:
            model->removeConnection(mAssociation[from].getName(), "out",
                                    mAssociation[to].getName(), "in");
            model->removeOutputPort(mAssociation[from].getName(), "out");
            model->removeInputPort(mAssociation[to].getName(), "in");

            mDeletedConnection.push_back(
                Connection(mAssociation[from].getName(),
                            "out",
                            mAssociation[to].getName(),
                            "in"));
            break;
        case CONNECTION_TYPE_IN_NAMED:
            model->removeConnection(mAssociation[from].getName(),
                                    mAssociation[to].getName(),
                                    mAssociation[to].getName(), "in");
            model->removeOutputPort(mAssociation[from].getName(),
                                    mAssociation[to].getName());
            model->removeInputPort(mAssociation[to].getName(), "in");

            mDeletedConnection.push_back(
                Connection(mAssociation[from].getName(),
                            mAssociation[to].getName(),
                            mAssociation[to].getName(),
                            "in"));
            break;
        case CONNECTION_TYPE_NAMED_OUT:
            model->removeConnection(mAssociation[from].getName(), "out",
                                    mAssociation[to].getName(),
                                    mAssociation[from].getName());
            model->removeOutputPort(mAssociation[from].getName(), "out");
            model->removeInputPort(mAssociation[to].getName(),
                                   mAssociation[from].getName());

            mDeletedConnection.push_back(
                Connection(mAssociation[from].getName(),
                            "out",
                            mAssociation[to].getName(),
                            mAssociation[from].getName()));
            break;
        case CONNECTION_TYPE_NAMED:
            model->removeConnection(mAssociation[from].getName(),
                                    mAssociation[to].getName(),
                                    mAssociation[to].getName(),
                                    mAssociation[from].getName());
            model->removeOutputPort(mAssociation[from].getName(),
                                    mAssociation[to].getName());
            model->removeInputPort(mAssociation[to].getName(),
                                   mAssociation[from].getName());

            mDeletedConnection.push_back(
                Connection(mAssociation[from].getName(),
                            mAssociation[to].getName(),
                            mAssociation[to].getName(),
                            mAssociation[from].getName()));
            break;
        }
    }

    void init(const vle::value::Map& buffer)
    {
        const vle::value::Map& init = vle::value::toMapValue(buffer);

        int nodeNumber = vle::value::toInteger(init.get("number"));
        if (nodeNumber <= 0) {
            throw vle::utils::ArgError("GraphTranslator: bad node number");
        } else {
            mCurrentMatrix.resize(0, 0);
            mCacheMatrix.resize(nodeNumber, nodeNumber);
            mAssociation.resize(mPrefix, nodeNumber);
        }

        if (init.exist("prefix")) {
            mPrefix = toString(init.get("prefix"));
            if (mPrefix.empty()) {
                throw vle::utils::ArgError("GraphTranslator: bad prefix");
            }
        }

        if (init.exist("port")) {
            std::string port = toString(init.get("port"));
            if (port == "in") {
                mConnectionType = CONNECTION_TYPE_IN_NAMED;
            } else if (port == "out") {
                mConnectionType = CONNECTION_TYPE_NAMED_OUT;
            } else if (port == "in-out") {
                mConnectionType = CONNECTION_TYPE_IN_OUT;
            } else {
                mConnectionType = CONNECTION_TYPE_NAMED;
            }
        }

        const vle::value::TupleValue adjmat =
            vle::value::toTuple(init.get("adjacency matrix"));

        if ((int)adjmat.size() != mCacheMatrix.cols() * mCacheMatrix.rows()) {
            throw vle::utils::ArgError(
                "GraphTranslator: problem in number of node");
        }

        std::copy(adjmat.begin(), adjmat.end(), mCacheMatrix.data());
        mCurrentMatrix = mCacheMatrix;

        if (init.exist("default classname")) {
            mDefaultClassName =
                vle::value::toString(init.get("default classname"));
        }

        if (init.exist("nodes")) {
            const vle::value::MapValue classes =
                vle::value::toMap(init.get("nodes"));

            for (vle::value::MapValue::const_iterator it = classes.begin();
                 it != classes.end(); ++it) {

                const std::string& classname = it->first;
                const vle::value::TupleValue& nodelst =
                    vle::value::toTuple(it->second);

                for (vle::value::TupleValue::const_iterator jt =
                     nodelst.begin(); jt != nodelst.end(); ++jt) {

                    mAssociation.replace(
                        mAssociation[*jt].getName(),
                        *jt,
                        classname);
                }
            }
        }
    }
};

DynamicGraphTranslator::DynamicGraphTranslator(const vle::value::Map& buffer)
    : mPimpl(new DynamicGraphTranslator::Pimpl(buffer))
{
}

DynamicGraphTranslator::~DynamicGraphTranslator()
{
    delete mPimpl;
}

int DynamicGraphTranslator::getNumber() const
{
    assert(mPimpl->mCacheMatrix.cols() == mPimpl->mCacheMatrix.rows());

    return mPimpl->mCacheMatrix.cols();
}

Eigen::MatrixXd DynamicGraphTranslator::getMatrix() const
{
    return mPimpl->mCacheMatrix;
}

void DynamicGraphTranslator::setMatrix(const Eigen::MatrixXd& matrix)
{
    assert(matrix.cols() == matrix.rows());
    assert(matrix.cols() == mPimpl->mCacheMatrix.cols());
    assert(matrix.cols() >= mPimpl->mCurrentMatrix.cols());

    mPimpl->mCacheMatrix = matrix;
}

void DynamicGraphTranslator::setConnectionType(ConnectionTypeOptions type)
{
    mPimpl->mConnectionType = type;
}

ConnectionTypeOptions DynamicGraphTranslator::getConnectionType() const
{
    return mPimpl->mConnectionType;
}

void DynamicGraphTranslator::setPrefix(const std::string& prefix)
{
    assert(not prefix.empty());

    mPimpl->mPrefix = prefix;
}

std::string DynamicGraphTranslator::getPrefix() const
{
    return mPimpl->mPrefix;
}

void DynamicGraphTranslator::setDefaultClassName(const std::string& name)
{
    assert(not name.empty());

    mPimpl->mDefaultClassName = name;
}

std::string DynamicGraphTranslator::getDefaultClassName() const
{
    return mPimpl->mDefaultClassName;
}

void DynamicGraphTranslator::build(vle::devs::Executive* model)
{
    assert(model);

    mPimpl->build(model);
}

void DynamicGraphTranslator::getNewModel(ModelNameList* list) const
{
    list->resize(mPimpl->mNewModel.size());

    std::copy(mPimpl->mNewModel.begin(),
              mPimpl->mNewModel.end(),
              list->begin());
}

void DynamicGraphTranslator::getDeletedModel(ModelNameList* list) const
{
    list->resize(mPimpl->mDeletedModel.size());

    std::copy(mPimpl->mDeletedModel.begin(),
              mPimpl->mDeletedModel.end(),
              list->begin());
}

void DynamicGraphTranslator::getNewConnection(ConnectionList* list) const
{
    list->resize(mPimpl->mNewConnection.size());

    std::copy(mPimpl->mNewConnection.begin(),
              mPimpl->mNewConnection.end(),
              list->begin());
}

void DynamicGraphTranslator::getDeletedConnection(ConnectionList* list) const
{
    list->resize(mPimpl->mDeletedConnection.size());

    std::copy(mPimpl->mDeletedConnection.begin(),
              mPimpl->mDeletedConnection.end(),
              list->begin());
}

