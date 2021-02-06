import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from scipy.optimize import leastsq
from sklearn.preprocessing import PolynomialFeatures
from sklearn import linear_model

def fit_func(p, x):
    f = np.poly1d(p)
    return f(x)

def err_func(p, x, y):
    ret = fit_func(p, x) - y
    return ret

def leastsqModel(X, Y):
    parameters = leastsq(err_func, p_init, args=(np.array(X), np.array(Y)))

n = 3
p_init = np.random.randn(n)


def multivariatePolynomialFitting(X, Y, X_new):
    #generate a model of polynomial features
    poly = PolynomialFeatures(degree=3)

    #transform the x data for proper fitting (for single variable type it returns,[1,x,x**2])
    X_ = poly.fit_transform(X)

    #transform the prediction to fit the model type
    #Y_ = poly.fit_transform(Y)

    #here we can remove polynomial orders we don't want for instance I'm removing the `x` component
    X_ = np.delete(X_,(1),axis=1)
    #Y_ = np.delete(Y_, (1), axis=1)
    
    #generate the regression object
    lr = linear_model.LinearRegression()

    #preform the actual regression
    model = lr.fit(X_, Y)

    #data = np.arange().reshape((12, 2))

    X_new_ = poly.fit_transform(X_new)
    X_new_ = np.delete(X_new_,(1),axis=1)
    pred_ = model.predict(X_new_)


    #fig = plt.figure()
    #ax = Axes3D(fig)
    #ax.scatter(X[:,0], X[:,1], Y)

    #ax.plot_surface(X[:,0], X[:,1], pred_)

    return model, pred_


def multivariateLogPolynomialFitting(X, Y, X_new):
    #generate a model of polynomial features
    poly = PolynomialFeatures(degree=3)

    lx = np.log2(X)                                                                                                                                                                                                                                    
    ly = np.log2(Y)                                                                                                                                                                                                                                  
    #input_x = list(map(lambda t: [t[0], t[1]], lx))

    #transform the x data for proper fitting (for single variable type it returns,[1,x,x**2])
    X_ = poly.fit_transform(lx)

    #transform the prediction to fit the model type
    #Y_ = poly.fit_transform(Y)

    #here we can remove polynomial orders we don't want for instance I'm removing the `x` component
    X_ = np.delete(X_,(1),axis=1)
    #Y_ = np.delete(Y_, (1), axis=1)
    
    #generate the regression object
    lr = linear_model.LinearRegression()

    #preform the actual regression
    model = lr.fit(X_, ly)

    #data = np.arange().reshape((12, 2))

    X_new_l = np.log2(X_new)

    X_new_l = poly.fit_transform(X_new_l)
    X_new_l = np.delete(X_new_l,(1),axis=1)
    pred_l = model.predict(X_new_l)

    pred_ = np.exp2(pred_l)

    #fig = plt.figure()
    #ax = Axes3D(fig)
    #ax.scatter(X[:,0], X[:,1], Y)

    #ax.plot_surface(X[:,0], X[:,1], pred_)

    return model, pred_

def multivariatePolynomialSklearn(X, Y):
    poly = PolynomialFeatures(degree=2)
    X_ = poly.fit_transform(X)

    X_train, X_test, Y_train, Y_test = train_test_split(X_, Y, test_size = 0.3, random_state = 4)

    lr = linear_model.LinearRegression()

    model = lr.fit(X_train, Y_train)
    score = model.score(X_test, Y_test)
    

    return model

# X = [[1,2], [2,6], [3,8], [4,1]]
# Y = [2, 7, 7, 2]

# m, p = multivariateLogPolynomialFitting(X, Y, X)

# print(p)