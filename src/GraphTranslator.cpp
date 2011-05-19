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
        for (int i = 0; i < mMatrix.rows(); ++i) {
            createNode(model, i);
        }

        for (int j = 0; j < mMatrix.cols(); ++j) {
            for (int i = 0; i < mMatrix.rows(); ++i) {
                if (mMatrix(j, i) > 0.0) {
                    createConnection(model, i, j);
                } else {
                    removeConnection(model, i, j);
                }
            }
        }
    }

    ConnectionTypeOptions mConnectionType;
    std::string mPrefix;
    std::string mDefaultClassName;
    detail::NodeList mAssociation;
    Eigen::MatrixXd mMatrix;

private:
    void createNode(vle::devs::Executive* model, int index)
    {
        const detail::Node& node = mAssociation[index];

        if (model->coupledmodel().findModel(node.getName()) == 0) {
            model->createModelFromClass(
                node.getClassName().empty() ? mDefaultClassName :
                node.getClassName(),
                node.getName());
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
            break;
        case CONNECTION_TYPE_IN_NAMED:
            model->addOutputPort(mAssociation[from].getName(),
                                 mAssociation[to].getName());
            model->addInputPort(mAssociation[to].getName(), "in");
            model->addConnection(mAssociation[from].getName(),
                                 mAssociation[to].getName(),
                                 mAssociation[to].getName(), "in");
            break;
        case CONNECTION_TYPE_NAMED_OUT:
            model->addOutputPort(mAssociation[from].getName(), "out");
            model->addInputPort(mAssociation[to].getName(),
                                mAssociation[from].getName());
            model->addConnection(mAssociation[from].getName(), "out",
                                 mAssociation[to].getName(),
                                 mAssociation[from].getName());
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
            break;
        }
    }

    void removeConnection(vle::devs::Executive* model, int to, int from)
    {
/*
 *        switch (mConnectionType) {
        case CONNECTION_TYPE_IN_OUT:
            model->addOutputPort(mAssociation[from].getName(), "out");
            model->addInputPort(mAssociation[to].getName(), "in");
            model->removeConnection(mAssociation[from].getName(), "out",
                                    mAssociation[to].getName(), "in");
            break;
        case CONNECTION_TYPE_IN_NAMED:
            model->addOutputPort(mAssociation[from].getName(),
                                 mAssociation[to].getName());
            model->addInputPort(mAssociation[to].getName(), "in");
            model->removeConnection(mAssociation[from].getName(),
                                    mAssociation[to].getName(),
                                    mAssociation[to].getName(), "in");
            break;
        case CONNECTION_TYPE_NAMED_OUT:
            model->addOutputPort(mAssociation[from].getName(), "out");
            model->addInputPort(mAssociation[to].getName(),
                                mAssociation[from].getName());
            model->removeConnection(mAssociation[from].getName(), "out",
                                    mAssociation[to].getName(),
                                    mAssociation[from].getName());
            break;
        case CONNECTION_TYPE_NAMED:
            model->addOutputPort(mAssociation[from].getName(),
                                 mAssociation[to].getName());
            model->addInputPort(mAssociation[to].getName(),
                                mAssociation[from].getName());
            model->removeConnection(mAssociation[from].getName(),
                                    mAssociation[to].getName(),
                                    mAssociation[to].getName(),
                                    mAssociation[from].getName());
            break;
        }
        */
    }

    void init(const vle::value::Map& buffer)
    {
        const vle::value::Map& init = vle::value::toMapValue(buffer);

        int nodeNumber = vle::value::toInteger(init.get("number"));
        if (nodeNumber <= 0) {
            throw vle::utils::ArgError("GraphTranslator: bad node number");
        } else {
            mMatrix.resize(nodeNumber, nodeNumber);
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

        if ((int)adjmat.size() != mMatrix.cols() * mMatrix.rows()) {
            throw vle::utils::ArgError(
                "GraphTranslator: problem in number of node");
        }

        std::copy(adjmat.begin(), adjmat.end(), mMatrix.data());

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

/*
 *
 * implementation of DynamicGraphTranslator
 *
 */

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
    assert(mPimpl->mMatrix.cols() == mPimpl->mMatrix.rows());

    return mPimpl->mMatrix.cols();
}

Eigen::MatrixXd DynamicGraphTranslator::getMatrix() const
{
    return mPimpl->mMatrix;
}

void DynamicGraphTranslator::setMatrix(const Eigen::MatrixXd& matrix)
{
    assert(matrix.cols() == matrix.rows());
    assert(matrix.cols() == mPimpl->mMatrix.cols());

    mPimpl->mMatrix = matrix;
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

