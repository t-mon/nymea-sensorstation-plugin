/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "sensordatafilter.h"

SensorDataFilter::SensorDataFilter(Type filterType, QObject *parent) :
    QObject(parent),
    m_filterType(filterType)
{

}

double SensorDataFilter::filterValue(double value)
{
    double resultValue =  value;
    switch (m_filterType) {
    case TypeLowPass:
        resultValue = lowPassFilterValue(value);
        break;
    case TypeHighPass:
        resultValue = highPassFilterValue(value);
        break;
    case TypeAverage:
        resultValue = averageFilterValue(value);
        break;
    }

    return resultValue;
}

bool SensorDataFilter::isReady() const
{
    // Note: filter is ready once 10% of window filled
    return m_inputData.size() >= m_filterWindowSize * 0.1;
}

void SensorDataFilter::reset()
{
    m_averageSum = 0;
    m_inputData.clear();
}

SensorDataFilter::Type SensorDataFilter::filterType() const
{
    return m_filterType;
}

QVector<double> SensorDataFilter::inputData() const
{
    return m_inputData;
}

QVector<double> SensorDataFilter::outputData() const
{
    return m_outputData;
}

uint SensorDataFilter::windowSize() const
{
    return m_filterWindowSize;
}

void SensorDataFilter::setFilterWindowSize(uint windowSize)
{
    Q_ASSERT_X(windowSize > 0, "value out of range", "The filter window size must be bigger than 0");
    m_filterWindowSize = windowSize;
}

double SensorDataFilter::lowPassAlpha() const
{
    return m_lowPassAlpha;
}

void SensorDataFilter::setLowPassAlpha(double alpha)
{
    Q_ASSERT_X(alpha > 0 && alpha <= 1, "value out of range", "The alpha low pass filter value must be [ 0 < alpha <= 1 ]");
    m_lowPassAlpha = alpha;
}

double SensorDataFilter::highPassAlpha() const
{
    return m_highPassAlpha;
}

void SensorDataFilter::setHighPassAlpha(double alpha)
{
    Q_ASSERT_X(alpha > 0 && alpha <= 1, "value out of range", "The alpha high pass filter value must be [ 0 < alpha <= 1 ]");
    m_highPassAlpha = alpha;
}

void SensorDataFilter::addInputValue(double value)
{
    m_inputData.append(value);
    if (m_inputData.size() > static_cast<int>(m_filterWindowSize)) {
        m_inputData.removeFirst();
    }
}

double SensorDataFilter::lowPassFilterValue(double value)
{
    addInputValue(value);

    // Check if we have enough data for filtering
    if (m_inputData.size() < 2) {
        return value;
    }

    QVector<double> outputData;
    outputData.append(m_inputData.at(0));
    for (int i = 1; i < m_inputData.size(); i++) {
        // y[i] := y[i-1] + α * (x[i] - y[i-1])
        outputData.append(outputData.at(i - 1) + m_lowPassAlpha * (m_inputData.at(i) - outputData.at(i - 1)));
    }

    m_outputData = outputData;

    return m_outputData.last();
}

double SensorDataFilter::highPassFilterValue(double value)
{
    addInputValue(value);

    // Check if we have enough data for filtering
    if (m_inputData.size() < 2) {
        return value;
    }

    QVector<double> outputData;
    outputData.append(m_inputData.at(0));
    for (int i = 1; i < m_inputData.size(); i++) {
        // y[i] := α * y[i-1] + α * (x[i] - x[i-1])
        outputData.append(m_highPassAlpha * outputData.at(i - 1) + m_highPassAlpha * (m_inputData.at(i) - m_inputData.at(i - 1)));
    }

    m_outputData = outputData;
    return m_outputData.last();
}

double SensorDataFilter::averageFilterValue(double value)
{
    if (m_inputData.isEmpty()) {
        addInputValue(value);
        m_averageSum = value;
        return value;
    }

    if (m_inputData.size() >= static_cast<int>(m_filterWindowSize)) {
        m_averageSum -= m_inputData.takeFirst();
    }

    addInputValue(value);
    m_averageSum += value;
    return m_averageSum / m_inputData.size();
}
