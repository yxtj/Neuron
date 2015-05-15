function [A,W,CM]=trainAW(rData,D,lambdaA,lambdaW,fDep=0,Ainit=0,Winit=0)
%%input parameters:
%rData:     raw data
%D:         delay matrix
%lambdaA:   regularization parameter for A
%lambdaW:   regularization parameter for A
%fDep:      factor for depolarization
%Ainit:     initial A to start with
%Winit:     initial W to start with

n=length(rData);

if(sum(size(Ainit)==[n n])!=2)
  Ainit=initAdjacency(n);
end
A=Ainit;
if(sum(size(Winit)==[n n])!=2)
  Winit=initWeight(n);
end
W=Winit;
if(nargout==3)
  CM=zeros(n,4);
end
for i=1:n
%  disp(sprintf('Working idx=%d',i));
  %[seq,cls]=mergeWithDelay(rData,i,D);
  %[X,y]=genDataByRef(n,seq,cls,rData(i));
  [X,y]=genDataFromRaw(rData,D,i,fDep);
  [A(:,i),W(:,i),J]=trainOneAW(i,X,y,Ainit(:,i),Winit(:,i),lambdaA,lambdaW);
  if(nargout==3)
    CM(i,:)=testOneAW(A(:,i),W(:,i),X,y);
  end
%  disp(sprintf('  error=%f\taccurancy=%f',J,(CM(i,1)+CM(i,4))/sum(CM(i,:))));
%  showCM(CM(i,:));
end
%showCM(sum(CM));

end
