function [weight,J]=trainOneW(idx,X,y,weight,lambda)

[J,gd]=costFunctionW(X,y, weight,lambda);
if(isnan(J))
  weight-=0.1*gd;
  clear gd;
end

options = optimset('GradObj', 'on', 'MaxIter', 400);
[weight, J, exit_flag] = fminunc(@(t)(costFunctionW(X, y, t, lambda)), weight, options);
weight(idx)=0;

end
